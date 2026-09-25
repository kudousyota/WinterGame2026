#include "CsvTable.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>

namespace
{
	//前後の空白を取り除く
	std::string Trim(const std::string& str)
	{
		const char* kSpace = " \t";
		size_t begin = str.find_first_not_of(kSpace);
		if (begin == std::string::npos)
		{
			return "";
		}
		size_t end = str.find_last_not_of(kSpace);
		return str.substr(begin, end - begin + 1);
	}

	//1行をカンマで分割する
	std::vector<std::string> Split(const std::string& line)
	{
		std::vector<std::string> tokens;
		std::istringstream iss(line);
		std::string token;
		while (std::getline(iss, token, ','))
		{
			tokens.push_back(Trim(token));
		}
		return tokens;
	}
}

void CsvTable::Load(const std::string& path)
{
	//先にクリアしとく
	m_path = path;
	m_columnIndex.clear();
	m_rows.clear();
	m_lineNumbers.clear();

	std::ifstream file(path);
	if (!file.is_open())
	{
		ReportError(-1, "", "CSVファイルが開けない");
		return;
	}

	bool isHeaderRead = false;
	int lineNumber = 0;
	std::string line;
	while (std::getline(file, line))
	{
		lineNumber++;

		//Windows改行対策
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}
		//ExcelでUTF-8保存したときに先頭に付くBOMを取り除く
		if (lineNumber == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
		{
			line.erase(0, 3);
		}
		if (line.empty())
		{
			continue;
		}

		//最初の行はヘッダー(列名)
		if (!isHeaderRead)
		{
			if (line[0] == '#')
			{
				line.erase(0, 1);
			}
			std::vector<std::string> columns = Split(line);
			for (int i = 0; i < static_cast<int>(columns.size()); i++)
			{
				if (columns[i].empty())
				{
					continue;
				}
				if (m_columnIndex.count(columns[i]) > 0)
				{
					ReportError(-1, columns[i], "列名が重複している");
					continue;
				}
				m_columnIndex[columns[i]] = i;
			}
			isHeaderRead = true;
			continue;
		}

		//2行目以降の#はコメント
		if (line[0] == '#')
		{
			continue;
		}

		m_rows.push_back(Split(line));
		m_lineNumbers.push_back(lineNumber);
	}

	if (!isHeaderRead)
	{
		ReportError(-1, "", "ヘッダー行がない(ファイルが空)");
	}
}

bool CsvTable::HasColumn(const std::string& column) const
{
	return m_columnIndex.count(column) > 0;
}

const std::string& CsvTable::GetString(int row, const std::string& column) const
{
	static const std::string kEmpty;
	const std::string* cell = GetCell(row, column);
	return cell ? *cell : kEmpty;
}

int CsvTable::GetInt(int row, const std::string& column) const
{
	const std::string* cell = GetCell(row, column);
	if (!cell)
	{
		return 0;
	}
	char* end = nullptr;
	long value = std::strtol(cell->c_str(), &end, 10);
	//最後まで数字として読めなかったら数値じゃない
	if (*end != '\0')
	{
		ReportError(row, column, "整数ではない(\"" + *cell + "\")");
		return 0;
	}
	return static_cast<int>(value);
}

float CsvTable::GetFloat(int row, const std::string& column) const
{
	const std::string* cell = GetCell(row, column);
	if (!cell)
	{
		return 0.0f;
	}
	char* end = nullptr;
	float value = std::strtof(cell->c_str(), &end);
	if (*end != '\0')
	{
		ReportError(row, column, "数値ではない(\"" + *cell + "\")");
		return 0.0f;
	}
	return value;
}

const std::string* CsvTable::GetCell(int row, const std::string& column) const
{
	if (row < 0 || row >= GetRowNum())
	{
		ReportError(-1, column, "行番号が範囲外(" + std::to_string(row) + ")");
		return nullptr;
	}
	auto it = m_columnIndex.find(column);
	if (it == m_columnIndex.end())
	{
		ReportError(-1, column, "ヘッダーにこの列名がない");
		return nullptr;
	}
	const std::vector<std::string>& cells = m_rows[row];
	if (it->second >= static_cast<int>(cells.size()) || cells[it->second].empty())
	{
		ReportError(row, column, "セルが空");
		return nullptr;
	}
	return &cells[it->second];
}

void CsvTable::ReportError(int row, const std::string& column, const std::string& message) const
{
	//例:Data/Csv/State.csv 2行目 [speed] 数値ではない("abc")
	std::string text = m_path;
	if (row >= 0 && row < GetRowNum())
	{
		text += " " + std::to_string(m_lineNumbers[row]) + "行目";
	}
	if (!column.empty())
	{
		text += " [" + column + "]";
	}
	text += " " + message;

	OutputDebugStringA(("CsvTable: " + text + "\n").c_str());
	MessageBoxA(nullptr, text.c_str(), "CSV読み込みエラー", MB_OK | MB_ICONERROR);
	assert(false && "CSV読み込みエラー");
}
