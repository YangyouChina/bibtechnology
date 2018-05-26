#pragma once

#include <vector>

using namespace std;

namespace RealtimeCurve{
	const int MaxPointsCount = 512;	// 曲线最大存储点数
	const int MaxLinesCount = 10;	// 控件容纳最大曲线个数

	class CLine
	{
	public:
		CLine(CString name=_T(""),
			int lineStyle = PS_SOLID, 
			COLORREF lineColor = RGB(255,0,0),
			int lineWidth = 2);
		CLine(const CLine& nLine);
		CLine& operator = (const CLine& nLine);
		~CLine();
		
		_declspec(property(get = getLineName,put = setLineName))	CString lineName;
		_declspec(property(get = getShowStatus,put = setShowStatus)) BOOL showStatus;
		_declspec(property(get = getLineColor,put = setLineColor)) COLORREF lineColor;
		_declspec(property(get = getLineType,put = setLineType)) int lineType;
		_declspec(property(get = getLineWidth,put = setLineWidth)) int lineWidth;

		void	AddPoint(float x, float y);
		float	GetPointX(int nIndex);
		float	GetPointY(int nIndex);
		int		GetPointCount();
		
		inline CString	getLineName()				{	return this->LineName;	}
		inline void		setLineName(CString name)	{	this->LineName = name;	}
		inline BOOL		getShowStatus()				{	return this->IsShow;	}
		inline void		setShowStatus(BOOL showStatus){	this->IsShow = showStatus;	}
		inline COLORREF getLineColor()				{	return this->LineColor;}
		inline void		setLineColor(COLORREF color){	this->LineColor = lineColor;	};
		inline int		getLineType()				{	return this->LineType;}
		inline void		setLineType(int lineType)	{	this->LineType = lineType;	};
		inline int		getLineWidth()				{	return this->LineWidth;}
		inline void		setLineWidth(int lineWidth)	{	this->LineWidth = lineWidth;	};

	public:
		CString		LineName;		// 曲线名称
		BOOL        IsShow;			// 曲线是否显示
		COLORREF	LineColor;		// 颜色
		int         LineType;		// 线型 solid,dash
		int         LineWidth;		// 线宽 pixel
		struct Point{
			float x;
			float y;
		};
	private:		
		int			m_currentSize;		// 有效的坐标点数
		Point *		m_pointArrayPtr;	// 存储曲线中Point的数组
		CRITICAL_SECTION g_cs ;
	};

	class CAxis
	{
	
	public:		
		struct AxisRangeStruct{		// 坐标轴范围
			float AxisMinValue;		// 轴最小值
			float AxisMaxValue;		// 轴最大值
		};
		
		CAxis(COLORREF color = RGB(0,255,0),
				int style = PS_SOLID, 
				int width = 2,
				float minValue = 0.0f, 
				float maxValue = 500.0);
		~CAxis(){	}
		CAxis(const CAxis& axis);
		CAxis &operator = (const CAxis& axis);

		inline void SetAxisRange(float minValue=0.0f,float maxValue=500.0f){
			m_axisRange.AxisMinValue = minValue;
			m_axisRange.AxisMaxValue = maxValue;
		}
		inline float GetAxisRange(){	
			return this->m_axisRange.AxisMaxValue-this->m_axisRange.AxisMinValue;
		}
		inline float GetRangeLowerLimit()	{	return this->m_axisRange.AxisMinValue;	}
		inline float GetRangeUpperLimit()	{	return this->m_axisRange.AxisMaxValue;	}
			
	public:
		BOOL        IsShow;			// 是否显示坐标轴
		COLORREF	AxisColor;		// 颜色
		int         AxisStyle;		// 线型
		int         AxisWidth;		// 线宽		

		int			CoorTextBoxWidth;	// 坐标文本框 宽度
		int			CoorTextBoxHeight;	// 坐标文本框 高度
	private:
		AxisRangeStruct m_axisRange;	// 轴代表值的范围
	};

	class CPlot :
		public CStatic
	{
		DECLARE_DYNAMIC(CPlot)

	public:
		CPlot(void);
		virtual ~CPlot(void);

		void Start();
		void Stop();
	
		
		void AddNewLine(CString name = _T(""),
					int style = PS_SOLID, 
					COLORREF color = RGB(255,0,0), 
					int iThick = 2);
		void AddNewPoint(float x,float y,int nLineIndex);	//向某一条曲线中添加数据
		CLine* GetLineByIndex(int nIndex);	// 得到第nIndex（从0开始）条曲线的引用
		CLine* GetLineByName(CString name);	// 根据曲线名称获取曲线
		int    GetLineCount();				// 获取当前有效曲线个数

		void SetRate(int nRate);
		void SetBkColor(COLORREF clrBkColor);
		CAxis& GetAxisY();
		CAxis& GetAxisX();

	private:
		void RefreshLayout();			// 刷新页面布局
		void RefreshPlot(CDC* pDC);		// 刷新控件
		void DrawBackground(CDC* pDC);	// 绘制背景
		void DrawAxises(CDC* pDC);		// 绘制坐标轴
		void DrawGrids(CDC* pDC);		// 绘制网格
		void DrawLine(CDC* pDC,const CRect& rectZone,const CPoint* pPtr,int nCount);
		void DrawLines(CDC* pDC);		// 绘制全部曲线
		void DrawLegend(CDC* pDC,CRect& rectZone);		// 绘制图例
		void DrawTile(CDC* pDC,CRect& rectZone);		// 绘制标题

		void ScaleProcess();		// 放大操作
		
		// 求两个矩形 rect1 和 rect2 的交集矩形
		CRect IntersectionRect(const CRect& rect1,const CRect& rect2);
		// 判断一个点是否在矩形上
		BOOL PointOnRect(const CRect& rect,const CPoint& point );
		// 判断一个点point是否在矩形rect内部
		BOOL PointInRect(const CRect& rect,const CPoint& point );	
		// 求两个点之间连线与rect边的交点
		CPoint IntersectionRectLine(const CRect& rect,const CPoint& inPoint,const CPoint& exPoint );
	public:
		//float		XMoveStep;		// 每次移动X轴增加的值

		BOOL        ShowGrid;		// 是否显示网格
		COLORREF	GridColor;		// 网格颜色
		int			GridStyle;		// 网格线型
		int			GridWidth;		// 网格线宽
		int			GridSize;		// 网格宽度		
		int			ShowTextGrap;	// 每个多个格显示一个坐标值
		COLORREF	BkGndColor;		// 背景颜色
		int			RefreshRate;	// 刷新图像周期，ms
	private:
		BOOL	m_bAdjustable;		// 是否需要自动调整
		// 布局相关
		CRect	m_rectCtrl;                 // 控件矩形区域
		CRect	m_rectPlot;                 // 绘图矩形区域
		CRect	m_rectTitle;				// 标题矩形区域
		CRect	m_rectLegend;				// 图例矩形区域
		int		m_marginLeft;				// 左侧留白宽度
		int		m_marginRight;				// 右侧留白宽度
		int		m_marginTop;				// 上部留白宽度
		int		m_marginBottom;				// 下部留白宽度
		int		m_titleHight;				// 标题高度
		int		m_legendHeight;				// 图例高度
		//	坐标轴相关
		CAxis m_axisX;		// X 轴
		CAxis m_axisY;      // Y 轴

		float		m_vppXAxis;		// 每个像素代表的X轴值
		float		m_vppYAxis;		// 每个像素点代表的Y轴值
		// 曲线相关
		CLine * m_linePtrArray[MaxLinesCount];	// 指针数组
		int validLinesCount;					// 有效线的个数
		// 缩放相关
		float m_originXLwLmt,m_originXUpLmt;	// 已经在自适应情况下的X，Y轴范围
		float m_originYLwLmt,m_originYUpLmt;	
		CPoint m_startPoint,m_endPoint;			// 鼠标左键起止点

	public:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnTimer(UINT nIDEvent);
		afx_msg void OnPaint();
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnLButtonDown(UINT nFlag,CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	
	};


}
