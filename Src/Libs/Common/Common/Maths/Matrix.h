#pragma once

//  Description:
//		 通用二维矩阵类
//================================================================================

//////////////////////////////////////////////////////////////////////////////////
//  Edit history:
//      (After updating this file(New, Add, Remove, Modify, Bugfix),
//       list here please.)
//================================================================================
//  ______________________________________________________________________________
//  |   |                Remark                         |             |          |
//  |No.|_______________________________________________|   Author    |   Date   |
//  |   |  Mode  |          Description                 |             |          |
//  +---+--------+--------------------------------------+-------------+----------+
//  |001| New    | Newly created.                       |   殷 威     |2017/02/23|
//  +---+--------+--------------------------------------+-------------+----------+
//  |002|        |                                      |             |          |
//  +----------------------------------------------------------------------------+
//________________________________________________________________________________
//////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <assert.h>
#include <string>
#include <vector>

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  矩阵类
 *
 * @tparam T
 */
/* ----------------------------------------------------------------------------*/
template<class T>
class CMatrix
{
public:
	CMatrix(){ m_nDot = 0; m_ppMatrix = NULL; m_matrixDim.cx = 0; m_matrixDim.cy = 0; m_bAlloc = FALSE; };
	CMatrix(SIZE dim);
	explicit CMatrix(int nRowDim,int nColDim);
	virtual ~CMatrix();
protected:
	SIZE   m_matrixDim;  //维度      cx:  列向维度    cy: 行向维度;
	T**	   m_ppMatrix;	//二维矩阵；
public:
	SIZE   Size()      { return m_matrixDim;		}
	
	CMatrix<T> &operator=( CMatrix<T> &other );
	/*
		重载了<< ,运算符;
		例：
		   CMatrix<double> matrix(3,3);
		   matrix<< 1,1,1,
		            2,1,3,
					4,-1,3;
	*/
	CMatrix<T> &operator<<(const T& data);
	CMatrix<T> &operator,(const T& data);
	T*			operator[](int index);

	CMatrix<T>&  operator+ (const CMatrix<T> &other);
	CMatrix<T>  operator* (CMatrix<T> &other);
	//格式化为单位矩阵；
	void  SetUnitMatrix();

	//设定维度；
	void  SetSize(int row,int col)
	{
		m_matrixDim.cx = col;
		m_matrixDim.cy = row;
		Make2DArray(m_ppMatrix,m_matrixDim.cy,m_matrixDim.cx);
	}

	//设定全0矩阵
	void  SetZeros(int row,int col)
	{
		m_matrixDim.cx = col;
		m_matrixDim.cy = row;
		Make2DArray(m_ppMatrix,m_matrixDim.cy,m_matrixDim.cx);
		for(int i = 0; i < row; i ++)
		{
			for(int j = 0; j < col; j ++)
			{
				m_ppMatrix[i][j] = 0;
			}
		}
	}
	//设定全1矩阵
	void  SetOnes(int row,int col)
	{
		m_matrixDim.cx = col;
		m_matrixDim.cy = row;
		Make2DArray(m_ppMatrix,m_matrixDim.cy,m_matrixDim.cx);
		for(int i = 0; i < row; i ++)
		{
			for(int j = 0; j < col; j ++)
			{
				m_ppMatrix[i][j] = 0;
			}
		}
	}


	//获取元素
	T	GetElementByIndex(int row,int col)
	{
		assert( row < m_matrixDim.cy && col < m_matrixDim.cx);
		return m_ppMatrix[row][col];
	}

	//设置元素
	void	SetElement(int row,int col, T element )
	{
		assert( row < m_matrixDim.cy && col < m_matrixDim.cx);
		m_ppMatrix[row][col] = element;
	}


protected:
	virtual bool AllocMem();	
private:
	int	 m_nDot;			//逗号个数
	bool m_bAlloc;			//是否已申请了内存；
};

template<class T>
CMatrix<T> CMatrix<T>::operator*(CMatrix<T> &other)
{
	SIZE matSize1 = m_matrixDim;
	SIZE matSize2 = other.Size();
	assert(matSize1.cx == matSize2.cy
		);
	CMatrix<T> matr(matSize1.cy,matSize2.cx);
 	for (int i = 0;i < matSize1.cy; i++ )		//行；
 	{
 		for (int k = 0;k < matSize2.cx; k++ )	//列；
 		{
 			matr[i][k] = (T)0.0f;
 			for (int j = 0;j < matSize1.cx; j++ )
 			{
 				matr[i][k] += m_ppMatrix[i][j] * other[j][k];
 			}
 		}
 	}
	return matr;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  申请内存
 *
 * @tparam T
 *
 * @Returns 是否申请成功  
 */
/* ----------------------------------------------------------------------------*/
template<class T>
bool CMatrix<T>::AllocMem()
{
	if (!m_bAlloc) {
		Make2DArray(m_ppMatrix,m_matrixDim.cy,m_matrixDim.cx);
		m_bAlloc = true;
		return true;
	}else{
		return false;
	}
}

typedef CMatrix<double>	CMatrixD;
typedef CMatrix<float>	CMatrixF;
typedef CMatrix<int>	CMatrixI;



/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  矩阵相加
 *
 * @tparam T
 * @Param mat1
 * @Param mat2
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
template<class T>
CMatrix<T>&  CMatrix<T>::operator+(const CMatrix<T> &other)
{
	SIZE matSize1 = m_matrixDim;
	SIZE matSize2 = other.Size();
	assert(matSize1.cx == matSize2.cx 
		&& matSize1.cy == matSize2.cy
		);
	CMatrix<T> matr(matSize1.cy,matSize2.cx);
	for (int i = 0; i < matSize1.cx; i++ )
	{
		for (int j=0; j < matSize1.cy; j++ )
		{
			m_ppMatrix[j][i] += other[j][i];
		}
	}
	return *this;
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  获取元素
 *
 * @tparam T
 * @Param index
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
template<class T>
T* CMatrix<T>::operator[]( int index )
{
	assert( index < m_matrixDim.cy );
	return reinterpret_cast<T*>(m_ppMatrix[index]);
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  便捷输入矩阵
 *
 * @tparam T
 * @Param data  值
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
template<class T>
CMatrix<T> & CMatrix<T>::operator,(const T& data )
{
	m_nDot++;
	m_ppMatrix[m_nDot/m_matrixDim.cx][m_nDot%m_matrixDim.cx] = data;						//行，列；
	return *this;
}
template<class T>
CMatrix<T> & CMatrix<T>::operator<<(const T& data )
{
	assert(NULL !=m_ppMatrix);
	m_nDot = 0;
	m_ppMatrix[m_nDot][m_nDot] = data;
	return *this;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  格式化单位矩阵
 *
 * @tparam T
 */
/* ----------------------------------------------------------------------------*/
template<class T>
void CMatrix<T>::SetUnitMatrix()
{
	assert(m_ppMatrix);
	int nMin = min(m_matrixDim.cx,m_matrixDim.cy);
	for ( int i =0; i< m_matrixDim.cx; i++ )
	{
		for ( int j =0; j< m_matrixDim.cy; j++ )
		{
			if ( i == j && i< nMin && j < nMin )
			{
				m_ppMatrix[j][i] = 1;
			}else
				m_ppMatrix[j][i] = 0;
		}
	}
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  构造函数
 *
 * @tparam T
 * @Param dim  维度
 */
/* ----------------------------------------------------------------------------*/
template<class T>
CMatrix<T>::CMatrix(SIZE dim)
{
	m_nDot	   = 0;
	m_matrixDim = dim;
	m_ppMatrix = NULL;
	Make2DArray(m_ppMatrix,m_matrixDim.cy,m_matrixDim.cx);
	m_bAlloc	= true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  赋值
 *
 * @tparam T
 * @Param other
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
template<class T>
CMatrix<T> & CMatrix<T>::operator=( CMatrix<T> &other )
{
	if ( m_ppMatrix )
	{
		Delete2DArray(m_ppMatrix,m_matrixDim.cx);
		m_bAlloc = false;
		m_matrixDim.cx = 0;
		m_matrixDim.cy = 0;
	}
	m_matrixDim = other.Size();
	Make2DArray(m_ppMatrix,m_matrixDim.cy,m_matrixDim.cx);
	m_bAlloc = true;
	for( int i =0; i< m_matrixDim.cy; i++ )
	{
		for(int j =0; j < m_matrixDim.cx; j++ )
		{
			m_ppMatrix[i][j] = other[i][j];
		}
	}
	return *this;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  析构函数
 *
 * @tparam T
 */
/* ----------------------------------------------------------------------------*/
template<class T>
CMatrix<T>::~CMatrix()
{
	Delete2DArray(m_ppMatrix,m_matrixDim.cy);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  构造函数
 *
 * @tparam T
 * @Param nRowDim   行维度
 * @Param nColDim   列维度
 */
/* ----------------------------------------------------------------------------*/
template<class T>
CMatrix<T>::CMatrix( int nRowDim,int nColDim )
{
	m_nDot	   = 0;
	m_matrixDim.cx = nColDim;
	m_matrixDim.cy = nRowDim;
	m_ppMatrix = NULL;
	Make2DArray(m_ppMatrix,nRowDim,nColDim);
	m_bAlloc	   = true;
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  创建矩阵空间
 *
 * @tparam T
 * @Param ppMatrix
 * @Param nRow
 * @Param nCol
 */
/* ----------------------------------------------------------------------------*/
template <class T>
void Make2DArray(T** &ppMatrix,int nRow,int nCol)
{
	ppMatrix = new T*[nRow];
	for ( int i=0; i<nRow; i++ )
	{
		ppMatrix[i] = new T[nCol];
	}
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  删除矩阵
 *
 * @tparam T
 * @Param ppMatrix
 * @Param nCol
 */
/* ----------------------------------------------------------------------------*/
template <class T>
void Delete2DArray( T** &ppMatrix,int nRow)
{
	if ( !ppMatrix )
	{
		return;
	}
	assert(ppMatrix && nRow > 0);
	for ( int i=0; i< nRow; i++ )
	{
		delete[] ppMatrix[i];
	}
	delete[] ppMatrix;
	ppMatrix = NULL;
}
