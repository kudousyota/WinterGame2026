#pragma once
#include <vector>
#include <string>
#include <unordered_map>

//ヘッダー付きCSVを読み込んで、列名で値を取り出せるようにするクラス
//・最初の行をヘッダー(列名)として扱う(先頭の#は無視する)
//・2行目以降の#で始まる行と空行はコメントとしてスキップ
//・列の順番が変わったり列が増えたりしても、列名で読むので壊れない
class CsvTable
{
public:
	//CSVを読み込む
	void Load(const std::string& path);

	//データ行の数(ヘッダー、コメント、空行は含まない)
	int GetRowNum() const { return static_cast<int>(m_rows.size()); }

	//列があるか(省略可能な列を読むときに使う)
	bool HasColumn(const std::string& column) const;

	//値の取得(row:データ行の番号、column:ヘッダーの列名)
	const std::string& GetString(int row, const std::string& column) const;
	int GetInt(int row, const std::string& column) const;
	float GetFloat(int row, const std::string& column) const;

private:
	//列名からセルの文字列を取得(列がない、セルが空ならエラー)
	const std::string* GetCell(int row, const std::string& column) const;

	//どのファイルの何行目の何列目が悪いかを表示して止める
	void ReportError(int row, const std::string& column, const std::string& message) const;

private:
	//読み込んだファイルのパス(エラー表示用)
	std::string m_path;
	//列名 → 何列目か
	std::unordered_map<std::string, int> m_columnIndex;
	//データ行(ヘッダーとコメントを除いたもの)
	std::vector<std::vector<std::string>> m_rows;
	//データ行がファイルの何行目か(エラー表示用)
	std::vector<int> m_lineNumbers;
};
