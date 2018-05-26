// LineChartCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "Plot.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define _CRTDBG_MAPALLOC

namespace RealtimeCurve {
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Line
	CLine::CLine(CString lineName, int lineStyle, COLORREF lineColor, int lineWidth)
		:IsShow(TRUE),
		m_currentSize(0),
		m_pointArrayPtr(NULL)
	{
		this->LineName = lineName;
		this->IsShow = TRUE;
		this->LineType = lineStyle;
		this->LineColor = lineColor;
		this->LineWidth = lineWidth;
		this->m_currentSize = 0;
		// 申请内存空间
		if (m_pointArrayPtr)
		{
			delete[] m_pointArrayPtr;
		}
		m_pointArrayPtr = new Point[MaxPointsCount];
		ASSERT(m_pointArrayPtr);
		InitializeCriticalSection(&g_cs);
	}

	CLine::CLine(const CLine &nLine)
	{
		*this = nLine;	// 调用等号重载
		InitializeCriticalSection(&g_cs);
	}

	CLine& CLine::operator=(const CLine& nLine)
	{
		if (this == &nLine) {// 等号右边是对象自身
			return *this;
		}
		// 拷贝基本属性
		this->LineName = nLine.LineName;
		this->IsShow = nLine.IsShow;
		this->LineType = nLine.LineType;
		this->LineColor = nLine.LineColor;
		this->LineWidth = nLine.LineWidth;
		this->m_currentSize = nLine.m_currentSize;
		// 数组空间
		// 申请内存空间
		if (m_pointArrayPtr)
		{
			delete[] m_pointArrayPtr;
		}
		m_pointArrayPtr = new Point[MaxPointsCount];
		ASSERT(m_pointArrayPtr);
		// 拷贝数组空间值
		for (int i = 0; i<MaxPointsCount; i++) {
			m_pointArrayPtr[i] = nLine.m_pointArrayPtr[i];
		}
		return *this;
	}

	CLine::~CLine()
	{
		// 释放空间
		if (m_pointArrayPtr)
		{
			delete[] m_pointArrayPtr;
			m_pointArrayPtr = NULL;
		}
		DeleteCriticalSection(&g_cs);
	}

	void CLine::AddPoint(float x, float y)
	{
		EnterCriticalSection(&g_cs);
		// 如果数组未满，则直接插入，否则移位后插入
		if (this->m_currentSize<MaxPointsCount)
		{
			this->m_currentSize++;
		}
		else {	// 移位
			for (int i = 0; i<this->m_currentSize - 1; i++) {
				this->m_pointArrayPtr[i] = this->m_pointArrayPtr[i + 1];
			}
		}
		// 插入新值
		this->m_pointArrayPtr[this->m_currentSize - 1].x = x;
		this->m_pointArrayPtr[this->m_currentSize - 1].y = y;

		LeaveCriticalSection(&g_cs);
	}

	float CLine::GetPointX(int nIndex)
	{
		ASSERT(nIndex<MaxPointsCount && nIndex >= 0);
		return (this->m_pointArrayPtr[nIndex].x);
	}

	float CLine::GetPointY(int nIndex)
	{
		ASSERT(nIndex<MaxPointsCount && nIndex >= 0);
		return (this->m_pointArrayPtr[nIndex].y);
	}

	int CLine::GetPointCount()
	{
		return (int)this->m_currentSize;
	}

	//////////////////////////////////////////////////////////////////////////
	// CAxis
	CAxis::CAxis(COLORREF color, int style, int width, float minValue, float maxValue) :
		IsShow(TRUE),
		CoorTextBoxWidth(40),
		CoorTextBoxHeight(20)
	{
		AxisColor = color;
		AxisStyle = style;
		AxisWidth = width;
		SetAxisRange(minValue, maxValue);
	}

	CAxis::CAxis(const CAxis& axis)
	{
		*this = axis;
	}

	CAxis & CAxis::operator=(const CAxis& axis)
	{
		if (this == &axis)
		{
			return *this;
		}
		IsShow = axis.IsShow;
		AxisColor = axis.AxisColor;
		AxisStyle = axis.AxisStyle;
		AxisWidth = axis.AxisWidth;

		CoorTextBoxWidth = axis.CoorTextBoxWidth;
		CoorTextBoxHeight = axis.CoorTextBoxHeight;

		m_axisRange = axis.m_axisRange;

		return *this;

	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	// CPlot

	IMPLEMENT_DYNAMIC(CPlot, CStatic)

		CPlot::CPlot(void)
	{
		//XMoveStep = 0.2f;		//每次X轴移动的值

		BkGndColor = RGB(0, 0, 0);			// 默认背景颜色
		m_axisX.AxisColor = RGB(0, 255, 0);	// 默认坐标轴颜色
		m_axisY.AxisColor = RGB(0, 255, 0);
		m_axisX.AxisWidth = 2;				// 默认坐标轴线宽
		m_axisY.AxisWidth = 2;
		RefreshRate = 500;				// 刷新频率
		m_bAdjustable = TRUE;

		ShowGrid = TRUE;			// 是否显示网格
		GridColor = RGB(64, 96, 64);	// 网格颜色
		GridStyle = PS_SOLID;		// 网格线型
		GridWidth = 1;		// 网格线宽
		GridSize = 10;		// 网格宽度
		m_vppXAxis = 0.01f;	// 每个像素代表的X轴值
		m_vppYAxis = 1.0f;	// 每个像素代表的Y轴值
		ShowTextGrap = 5;	// 每隔多个格显示一个坐标值

		m_marginLeft = 50;
		m_marginRight = 0;
		m_marginTop = 20;
		m_marginBottom = 20;

		m_legendHeight = m_marginTop - 2;
		m_titleHight = m_marginTop - 2;

		validLinesCount = 0;
		// 数组指针赋空值
		for (int i = 0; i<10; i++)
		{
			m_linePtrArray[i] = NULL;
		}
	}

	CPlot::~CPlot(void)
	{
		// 释放空间
		for (int i = 0; i<10; i++)
		{
			if (m_linePtrArray[i])
			{
				delete m_linePtrArray[i];
			}
		}
	}

	//
	void CPlot::Start()
	{
		if (RefreshRate > 0)
		{
			SetTimer(1, RefreshRate, NULL);
		}
	}

	void CPlot::Stop()
	{
		KillTimer(1);
	}

	void CPlot::AddNewLine(CString name, int style, COLORREF color, int width)
	{
		m_linePtrArray[validLinesCount] = new CLine(name, style, color, width);
		validLinesCount++;
	}

	void CPlot::AddNewPoint(float x, float y, int nLineIndex)
	{
		CLine* nLinePtr = GetLineByIndex(nLineIndex);
		nLinePtr->AddPoint(x, y);
		m_axisX.SetAxisRange(x - 100.0f, x);
		// 记录X轴的范围
		m_originXLwLmt = x - 100.0f;
		m_originXUpLmt = x;
		// 自动调整纵坐标轴的范围
		if (m_bAdjustable)
		{
			// 调整Y轴
			float newLowerLmt = m_axisY.GetRangeLowerLimit();
			float newUpperLmt = m_axisY.GetRangeUpperLimit();
			if (y>newUpperLmt)
			{
				newUpperLmt = y;
			}
			if (y<newLowerLmt) {
				newLowerLmt = y;
			}
			m_axisY.SetAxisRange(newLowerLmt, newUpperLmt);
			// 记录原始Y轴范围
			m_originYLwLmt = newLowerLmt;
			m_originYUpLmt = newUpperLmt;
		}
	}


	CLine* CPlot::GetLineByIndex(int nIndex)
	{
		if (nIndex >= 0 && nIndex<MaxLinesCount)
		{
			return (m_linePtrArray[nIndex]);
		}
		else {
			return NULL;
		}
	}


	CLine* CPlot::GetLineByName(CString name)
	{
		for (int i = 0; i<validLinesCount; i++)
		{
			if (name.Compare(m_linePtrArray[i]->getLineName()) == 0) {
				return m_linePtrArray[i];
			}
		}
		return NULL;
	}

	int CPlot::GetLineCount()
	{
		return validLinesCount;
	}

	//
	void CPlot::SetRate(int nRate)
	{
		RefreshRate = nRate;
	}

	void CPlot::SetBkColor(COLORREF clrBkColor)
	{
		BkGndColor = clrBkColor;
	}
	//

	//
	CAxis& CPlot::GetAxisY()
	{
		return m_axisY;
	}

	CAxis& CPlot::GetAxisX()
	{
		return m_axisX;
	}

	void CPlot::RefreshLayout()
	{
		// 更新空间区域
		GetClientRect(&m_rectCtrl);
		// 计算新的绘制曲线区域
		m_rectPlot.left = m_rectCtrl.left + m_marginLeft;
		m_rectPlot.right = m_rectCtrl.right - m_marginRight;
		m_rectPlot.top = m_rectCtrl.top + m_marginTop;
		m_rectPlot.bottom = m_rectCtrl.bottom - m_marginBottom;
		// 标题区域
		m_rectTitle.left = m_rectPlot.left;
		m_rectTitle.right = m_rectTitle.left + m_rectPlot.Width() / 2;
		m_rectTitle.top = m_marginTop / 2 - m_titleHight / 2;
		m_rectTitle.bottom = m_marginTop / 2 + m_titleHight / 2;
		// 图例区域
		m_rectLegend.left = m_rectTitle.right + 2;
		m_rectLegend.right = m_rectPlot.right;
		m_rectLegend.top = m_marginTop / 2 - m_legendHeight / 2;
		m_rectLegend.bottom = m_marginTop / 2 + m_legendHeight / 2;
		//	计算每像素代表的坐标值
		m_vppXAxis = (m_axisX.GetAxisRange()) / m_rectPlot.Width();
		m_vppYAxis = (m_axisY.GetAxisRange()) / m_rectPlot.Height();
	}

	void CPlot::RefreshPlot(CDC* pDC)
	{
		//更新布局
		RefreshLayout();
		//绘制背景
		DrawBackground(pDC);
		//绘制坐标轴
		DrawAxises(pDC);
		//绘制网格
		DrawGrids(pDC);
		//绘制图例
		DrawLegend(pDC, m_rectLegend);
		//绘制标题
		DrawTile(pDC, m_rectTitle);
		//绘制曲线
		DrawLines(pDC);
	}

	void CPlot::DrawBackground(CDC* pDC)
	{
		pDC->FillSolidRect(&m_rectCtrl, BkGndColor);
	}

	void CPlot::DrawAxises(CDC* pDC)
	{
		if (!pDC->GetSafeHdc()) { return; }
		// 绘制X轴
		if (m_axisX.IsShow)
		{
			CPen pen(m_axisX.AxisStyle, m_axisX.AxisWidth, m_axisX.AxisColor);
			CPen *oldPen = pDC->SelectObject(&pen);
			// 下方X轴
			pDC->MoveTo(m_rectPlot.left, m_rectPlot.bottom);
			pDC->LineTo(m_rectPlot.right, m_rectPlot.bottom);
			// 上方X轴
			pDC->MoveTo(m_rectPlot.left, m_rectPlot.top);
			pDC->LineTo(m_rectPlot.right, m_rectPlot.top);
			// 恢复画笔
			pDC->SelectObject(oldPen);
		}
		// 绘制Y轴
		if (m_axisY.IsShow)
		{
			CPen pen(m_axisY.AxisStyle, m_axisY.AxisWidth, m_axisY.AxisColor);
			CPen *oldPen = pDC->SelectObject(&pen);
			// 绘制横线
			pDC->MoveTo(m_rectPlot.left, m_rectPlot.bottom);
			pDC->LineTo(m_rectPlot.left, m_rectPlot.top);
			// 恢复画笔
			pDC->SelectObject(oldPen);
		}
	}

	void CPlot::DrawGrids(CDC* pDC)
	{
		if ((!pDC->GetSafeHdc()) || (!this->ShowGrid))
		{
			return;
		}

		CString txtStr;	// 显示坐标值的文字
		CFont textFont;	// 坐标值文字格式
						// 坐标值文字格式
		textFont.CreatePointFont(80, _T("Courier New"), NULL);
		// 绘制纵向网格
		CPen pen(this->GridStyle, this->GridWidth, this->GridColor);
		CPen *oldPen = pDC->SelectObject(&pen);
		for (int i = 0, xi = 0;
			xi = i*this->GridSize + m_rectPlot.left, xi <= m_rectPlot.right;
			i++)
		{
			if (i>0) {	//与Y轴重合的不用绘制
				pDC->MoveTo(xi, m_rectPlot.bottom);
				pDC->LineTo(xi, m_rectPlot.top);
			}

			// 显示X坐标值
			if (i%ShowTextGrap == 0)
			{
				CFont *oldFont = pDC->SelectObject(&textFont);// 设置字体格式
				pDC->SetTextColor(m_axisX.AxisColor);	// 设置字体颜色				
				txtStr.Format(_T("%4.1f"), m_axisX.GetRangeLowerLimit() + i*m_vppXAxis*GridSize);
				pDC->DrawText(txtStr, CRect(xi - m_axisX.CoorTextBoxWidth / 2,
					m_rectPlot.bottom + 1,	// +1是为了不挡住X坐标轴
					xi + m_axisX.CoorTextBoxWidth / 2,
					m_rectPlot.bottom + m_axisX.CoorTextBoxHeight + 1),
					DT_BOTTOM | DT_CENTER);
				pDC->SelectObject(oldFont);
			}
		}

		// 绘制横向网格
		for (int i = 0, yi = 0;
			yi = m_rectPlot.bottom - i*this->GridSize, yi>m_rectPlot.top;
			i++)
		{
			if (i != 0)// 与X轴重合的不用绘制
			{
				pDC->MoveTo(m_rectPlot.left, yi);
				pDC->LineTo(m_rectPlot.right, yi);
			}
			// 显示Y坐标值
			if (i%ShowTextGrap == 0)
			{
				CFont *oldFont = pDC->SelectObject(&textFont);// 设置字体格式
				pDC->SetTextColor(m_axisY.AxisColor);	// 设置字体颜色				
				txtStr.Format(_T("%4.1f"), m_axisY.GetRangeLowerLimit() + i*m_vppYAxis*GridSize);
				pDC->DrawText(txtStr, CRect(m_rectCtrl.left,
					yi - m_axisY.CoorTextBoxHeight / 2,
					m_rectPlot.left - 1,
					yi + m_axisY.CoorTextBoxHeight / 2),
					DT_BOTTOM | DT_RIGHT);
				pDC->SelectObject(oldFont);
			}
		}
		// 恢复画笔
		pDC->SelectObject(oldPen);
	}

	void CPlot::DrawLine(CDC* pDC, const CRect& rectZone, const CPoint* pPtr, int nCount)
	{
		if (nCount<1 || pPtr == NULL) {	// 无需绘制
			return;
		}
		BOOL lastPointInRect = FALSE;	// 上一个点是否在矩形内
		CPoint tpPoint;	//临时点
						// 逐点绘制
		for (int i = 0; i<nCount; i++) {
			if (i == 0) {//第一个点
				lastPointInRect = PointInRect(rectZone, pPtr[i]);
				pDC->MoveTo(pPtr[i].x, pPtr[i].y);
				continue;;
			}
			if (lastPointInRect == FALSE
				&& PointInRect(rectZone, pPtr[i]) == FALSE)
			{// 上一个点和该点均不在矩形内
				continue;
			}
			if (lastPointInRect == TRUE
				&& PointInRect(rectZone, pPtr[i]))
			{// 上一点和该点均在矩形内
				pDC->LineTo(pPtr[i].x, pPtr[i].y);
				continue;
			}
			if (lastPointInRect) {// 上一个点在，该点不在
				tpPoint = IntersectionRectLine(rectZone, pPtr[i - 1], pPtr[i]);
				pDC->MoveTo(pPtr[i - 1].x, pPtr[i - 1].y);
				pDC->LineTo(tpPoint.x, tpPoint.y);
				lastPointInRect = FALSE;
			}
			else {
				tpPoint = IntersectionRectLine(rectZone, pPtr[i], pPtr[i - 1]);
				pDC->MoveTo(tpPoint.x, tpPoint.y);
				pDC->LineTo(pPtr[i].x, pPtr[i].y);
				lastPointInRect = TRUE;
			}

		}

		//pDC->MoveTo(pPtr[0].x,pPtr[0].y);
		//for (int i=1;i<nCount;i++)
		//{
		//	pDC->LineTo(pPtr[i].x,pPtr[i].y);
		//}
	}

	void CPlot::DrawLines(CDC *pDC)
	{
		// 遍历每一条曲线
		for (int i = 0; i<GetLineCount(); i++)
		{
			CLine* nLine = GetLineByIndex(i);
			//
			if (!nLine->IsShow || nLine->GetPointCount()<1) {
				continue;
			}
			// 动态申请临时数组
			CPoint* nCPointArray = new CPoint[nLine->GetPointCount()];
			// 画笔
			CPen pen(nLine->LineType, nLine->LineWidth, nLine->LineColor);
			CPen *oldPen = pDC->SelectObject(&pen);
			// 坐标转化与处理
			for (int k = 0; k<nLine->GetPointCount(); k++)
			{
				// 将浮点数Point坐标数据转为整数
				int x = (int)(m_rectPlot.right - ((m_axisX.GetRangeUpperLimit() - nLine->GetPointX(k)) / (m_axisX.GetAxisRange())*m_rectPlot.Width()));
				int y = (int)(m_rectPlot.bottom - ((nLine->GetPointY(k) - m_axisY.GetRangeLowerLimit()) / (m_axisY.GetAxisRange())*m_rectPlot.Height()));
				// 将转化后的值存入数组
				nCPointArray[k].x = x;
				nCPointArray[k].y = y;
			}
			// 绘制单条曲线
			DrawLine(pDC, &(this->m_rectPlot), nCPointArray, nLine->GetPointCount());
			// 释放数组空间
			if (nCPointArray) {
				delete[] nCPointArray;
			}
			// 恢复画笔
			pDC->SelectObject(oldPen);
		}
	}

	void CPlot::DrawLegend(CDC* pDC, CRect &rectZone)
	{
		if (!pDC->GetSafeHdc()) { return; }
		int lineLength = 30;			// 图例线长
		int textWidht = 20;			// 图例字体宽度
		int unitGap = 10;			// 两个图例间的距离
		int xCoord = rectZone.left;// 当前X轴坐标
								   // 遍历所有曲线
		for (int i = 0; i<validLinesCount && xCoord<rectZone.right; i++)
		{
			CPen pen(m_linePtrArray[i]->getLineType(), m_linePtrArray[i]->getLineWidth(), m_linePtrArray[i]->getLineColor());
			CPen *oldPen = pDC->SelectObject(&pen);
			pDC->MoveTo(xCoord, rectZone.top + rectZone.Height() / 2);
			xCoord += lineLength;
			pDC->LineTo(xCoord, rectZone.top + rectZone.Height() / 2);
			xCoord += 5;// 向右移动5个像素点
			pDC->SelectObject(oldPen);
			CFont font;
			font.CreatePointFont(90, _T("Courier New"), pDC);
			CFont *oldFont = pDC->SelectObject(&font);
			pDC->SetTextColor(m_linePtrArray[i]->getLineColor());
			pDC->DrawText(m_linePtrArray[i]->getLineName(),
				CRect(xCoord, rectZone.top, xCoord + textWidht, rectZone.bottom),
				DT_CENTER);
			xCoord += (textWidht + unitGap);
			pDC->SelectObject(oldFont);
		}
	}

	void CPlot::DrawTile(CDC* pDC, CRect &rectZone)
	{
		if (!pDC->GetSafeHdc()) { return; }
		CFont font;
		font.CreatePointFont(90, _T("Courier New"), pDC);
		CFont *oldFont = pDC->SelectObject(&font);
		pDC->SetTextColor(RGB(255, 255, 0));
		pDC->DrawText(_T("BLT Temperature"), rectZone, DT_CENTER);
		pDC->SelectObject(oldFont);
	}

	void CPlot::ScaleProcess()
	{
		CRect rect;	// 鼠标拖动的区域
		if (m_startPoint.x>m_endPoint.x) {
			rect.left = m_endPoint.x;
			rect.right = m_startPoint.x;
		}
		else {
			rect.left = m_startPoint.x;
			rect.right = m_endPoint.x;
		}
		if (m_startPoint.y>m_endPoint.y)
		{
			rect.top = m_endPoint.y;
			rect.bottom = m_startPoint.y;
		}
		else {
			rect.top = m_startPoint.y;
			rect.bottom = m_endPoint.y;
		}
		// rect 与 m_rectPlot的交集
		CRect rectScale = IntersectionRect(rect, m_rectPlot);

		// 选择区域过小，不进行缩放
		if (rectScale.Width()<10 || rectScale.Height()<10) {
			return;
		}
		// 缩放处理
		float newXAxisLwLmt, newXAxisUpLmt;
		float newYAxisLwLmt, newYAxisUpLmt;
		newXAxisLwLmt = m_axisX.GetRangeLowerLimit()
			+ (rectScale.left - m_rectPlot.left)*m_axisX.GetAxisRange() / m_rectPlot.Width();
		newXAxisUpLmt = m_axisX.GetRangeLowerLimit()
			+ (rectScale.right - m_rectPlot.left)*m_axisX.GetAxisRange() / m_rectPlot.Width();
		newYAxisLwLmt = m_axisY.GetRangeLowerLimit()
			+ (m_rectPlot.bottom - rectScale.bottom)*m_axisY.GetAxisRange() / m_rectPlot.Height();
		newYAxisUpLmt = m_axisY.GetRangeLowerLimit()
			+ (m_rectPlot.bottom - rectScale.top)*m_axisY.GetAxisRange() / m_rectPlot.Height();
		m_axisX.SetAxisRange(newXAxisLwLmt, newXAxisUpLmt);
		m_axisY.SetAxisRange(newYAxisLwLmt, newYAxisUpLmt);
		RefreshLayout();
		m_bAdjustable = FALSE;
	}

	CRect CPlot::IntersectionRect(const CRect& rect1, const CRect& rect2)
	{
		CRect interRect;	//交集矩形
		interRect.left = (rect1.left<rect2.left) ? (rect2.left) : (rect1.left);
		interRect.right = (rect1.right<rect2.right) ? (rect1.right) : (rect2.right);
		interRect.top = (rect1.top<rect2.top) ? (rect2.top) : (rect1.top);
		interRect.bottom = (rect1.bottom<rect2.bottom) ? (rect1.bottom) : (rect2.bottom);
		return interRect;
	}

	BOOL CPlot::PointOnRect(const CRect& rect, const CPoint& point)
	{
		if (rect.left == point.x
			|| rect.right == point.x
			|| rect.top == point.y
			|| rect.bottom == point.y)
		{
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	BOOL CPlot::PointInRect(const CRect& rect, const CPoint& point)
	{
		if (rect.left <= point.x
			&& point.x <= rect.right
			&& rect.top <= point.y
			&& point.y <= rect.bottom)
		{
			return TRUE;

		}
		else {
			return FALSE;
		}
	}

	CPoint CPlot::IntersectionRectLine(const CRect& rect, const CPoint& inPoint, const CPoint& exPoint)
	{
		CPoint intersectionPoint;
		CRect constrRect;	//以inPoint 和 exPoint构造一个矩形
		float t;		// 相似三角形的比例
		constrRect.left = (inPoint.x<exPoint.x) ? (inPoint.x) : (exPoint.x);
		constrRect.right = (inPoint.x<exPoint.x) ? (exPoint.x) : (inPoint.x);
		constrRect.top = (inPoint.y<exPoint.y) ? (inPoint.y) : (exPoint.y);
		constrRect.bottom = (inPoint.y<exPoint.y) ? (exPoint.y) : (inPoint.y);
		constrRect = IntersectionRect(constrRect, rect);	// 求两个矩形交集
		if (constrRect.left == rect.left)
		{
			intersectionPoint.x = rect.left;
			t = ((float)(inPoint.x - intersectionPoint.x)) / (inPoint.x - exPoint.x);
			intersectionPoint.y = (int)((inPoint.y + t*exPoint.y) / (t + 1));
		}
		else if (constrRect.right == rect.right)
		{
			intersectionPoint.x = rect.right;
			t = ((float)(inPoint.x - intersectionPoint.x)) / (inPoint.x - exPoint.x);
			intersectionPoint.y = (int)((inPoint.y + t*exPoint.y) / (t + 1));
		}
		else if (constrRect.top == rect.top)
		{
			intersectionPoint.y = rect.top;
			t = ((float)(inPoint.y - intersectionPoint.y)) / (inPoint.y - exPoint.y);
			intersectionPoint.x = (int)((inPoint.x + t*exPoint.x) / (t + 1));

		}
		else {
			intersectionPoint.y = rect.bottom;
			t = ((float)(inPoint.y - intersectionPoint.y)) / (inPoint.y - exPoint.y);
			intersectionPoint.x = (int)((inPoint.x + t*exPoint.x) / (t + 1));
		}

		return intersectionPoint;

	}

	BEGIN_MESSAGE_MAP(CPlot, CStatic)
		ON_WM_TIMER()
		ON_WM_PAINT()
		ON_WM_ERASEBKGND()
		ON_WM_SIZE()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONUP()
		ON_WM_LBUTTONDBLCLK()
	END_MESSAGE_MAP()

	// CPlot message handlers

	void CPlot::OnTimer(UINT nIDEvent)
	{
		Invalidate(TRUE);
		CStatic::OnTimer(nIDEvent);
	}

	void CPlot::OnPaint()
	{
		CPaintDC dc(this); // device context for painting
		CDC *pDC = GetDC();
		CRect rect;
		GetClientRect(&rect);

		//创建一个内存中的显示设备
		CDC nDC;
		nDC.CreateCompatibleDC(NULL);

		//创建一个内存中的图像
		CBitmap nBitmap;
		nBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());

		//指定内存显示设备在内存中的图像上画图
		nDC.SelectObject(&nBitmap);

		// 刷新控件
		RefreshPlot(&nDC);

		//将内存中画好的图像直接拷贝到屏幕指定区域上
		pDC->BitBlt(rect.left, rect.top, rect.right, rect.bottom, &nDC, 0, 0, SRCCOPY);

		//释放相关资源
		nBitmap.DeleteObject();
		nDC.DeleteDC();
		ReleaseDC(pDC);
	}

	BOOL CPlot::OnEraseBkgnd(CDC* pDC)
	{
		//return TRUE;
		return CStatic::OnEraseBkgnd(pDC);
	}

	void CPlot::OnSize(UINT nType, int cx, int cy)
	{
		CStatic::OnSize(nType, cx, cy);
		RefreshLayout();
		Invalidate();
	}

	void CPlot::OnLButtonDown(UINT nFlag, CPoint point)
	{
		m_startPoint = point;
		TRACE("Left Button Down.Point Coordinate:x=%d,y=%d.\n", point.x, point.y);
		CStatic::OnLButtonDown(nFlag, point);
	}

	void CPlot::OnLButtonUp(UINT nFlag, CPoint point)
	{
		m_endPoint = point;
		ScaleProcess();	// 缩放处理
		OnPaint();		// 重绘
		TRACE("Left Button Up.Point Coordinate:x=%d,y=%d.\n", point.x, point.y);
		CStatic::OnLButtonUp(nFlag, point);
	}

	void CPlot::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		m_bAdjustable = TRUE;	// 重新进行自调整
		m_axisX.SetAxisRange(m_originXLwLmt, m_originXUpLmt);
		m_axisY.SetAxisRange(m_originYLwLmt, m_originYUpLmt);
		OnPaint();				// 重绘
		CStatic::OnLButtonDblClk(nFlags, point);
	}

}
