#pragma once

float dot(const float* a, const float* b, int n)
{
	if (!a || !b) throw std::runtime_error("null 포인터");
	if (n < 0)    throw std::runtime_error("잘못된 길이");

	float result = 0.0f;
	for (int i = 0; i < n; ++i)
		result += a[i] * b[i];

	return result;
}

struct Matrix
{
public:
	int row, col;
	float** map;

	Matrix(std::initializer_list<std::initializer_list<float>> list)
	{
		row = list.size();
		col = row ? list.begin()->size() : 0;

		for (auto& r : list)
			if ((int)r.size() != col) throw std::runtime_error("행 길이 불일치");

		map = new float*[row];
		int r = 0;
		for (auto& row : list)
		{
			map[r] = new float[this->col];
			int c = 0;
			for (int val : row)
				map[r][c++] = val;
			++r;
		}
	}

	Matrix(float** arr, int r, int c)
		: map(arr), row(r), col(c)
	{

	}

	// 전치 행렬
	Matrix Transpose()
	{
		float** result;

		result = new float*[col];

		for (int i = 0; i < col; i++)
			result[i] = new float[row];

		for (int i = 0; i < col; i++)
		{
			for (int j = 0; j < row; j++)
			{
				result[i][j] = this->map[j][i];
			}
		}

		return Matrix(result, col, row);
	}

	Matrix operator+(const Matrix& other) const
	{
		if (row != other.row || col != other.col)
			throw std::runtime_error("크기 불일치");

		float** temp = this->map;
		
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
				temp[i][j] += other.map[i][j];
		}

		return Matrix(temp, row, col);
	}

	Matrix operator-(const Matrix& other) const
	{
		if (row != other.row || col != other.col)
			throw std::runtime_error("크기 불일치");

		float** temp = this->map;

		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
				temp[i][j] -= other.map[i][j];
		}

		return Matrix(temp, row, col);
	}

	Matrix operator*(Matrix& other) const
	{
		if (col != other.row)
			throw std::runtime_error("크기 불일치");

		float** temp;

		temp = new float*[row];

		for (int i = 0; i < row; ++i)
		{
			temp[i] = new float [other.col];
		}

		Matrix trans = other.Transpose();

		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < other.col; j++)
				temp[i][j] = dot(this->map[i], trans.map[j], col);
		}

		return Matrix(temp, row, other.col);
	}
};