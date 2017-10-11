
// electromagneticDlg.cpp : implementation file
//

#include "stdafx.h"
#include "electromagnetic.h"
#include "electromagneticDlg.h"
#include "afxdialogex.h"

#include <list>

#include "model.h"

using namespace plot;
using namespace math;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_INVOKE WM_USER + 1234

// CelectromagneticDlg dialog

const double c1 = -1,
             c2 = 1;

v3 v0 = { 1, 0, 0 },
   e0 = { 0, 0, 0 },
   b0 = { 0, 0, 1 };
double dt = 0.01, t = 0;

using points_t = std::list < point < double > > ;

simple_list_plot < points_t > x_of_t, y_of_t, z_of_t, r_of_t;
simple_list_plot < points_t > vx_of_t, vy_of_t, vz_of_t, v_of_t;

UINT SimulationThreadProc(LPVOID pParam)
{
    CelectromagneticDlg & dlg = * (CelectromagneticDlg *) pParam;
    double _t = 0, _dt;
    v3 _v0, _e0, _b0;
    dlg.Invoke([&] ()
    {
        _v0 = v0; _e0 = e0; _b0 = b0; _dt = dt;
    });
    dresult3 x_dx = { { 0, 0, 0 }, _v0 };
    dfunc3_t system = model::make_electromagnetic_dfunc(_e0, _b0, c1, c2);
    size_t iteration = 0;
    r_of_t.data->clear();
    x_of_t.data->clear();
    y_of_t.data->clear();
    z_of_t.data->clear();
    v_of_t.data->clear();
    vx_of_t.data->clear();
    vy_of_t.data->clear();
    vz_of_t.data->clear();
    while (dlg.m_bWorking)
    {
        if ((iteration % 20) == 0)
        {
            r_of_t.data->emplace_back(_t, norm(x_dx.x));
            x_of_t.data->emplace_back(_t, x_dx.x.x);
            y_of_t.data->emplace_back(_t, x_dx.x.y);
            z_of_t.data->emplace_back(_t, x_dx.x.z);
            v_of_t.data->emplace_back(_t, norm(x_dx.dx));
            vx_of_t.data->emplace_back(_t, x_dx.dx.x);
            vy_of_t.data->emplace_back(_t, x_dx.dx.y);
            vz_of_t.data->emplace_back(_t, x_dx.dx.z);
            if (r_of_t.data->size() > 100)
            {
                r_of_t.data->pop_front();
                x_of_t.data->pop_front();
                y_of_t.data->pop_front();
                z_of_t.data->pop_front();
                v_of_t.data->pop_front();
                vx_of_t.data->pop_front();
                vy_of_t.data->pop_front();
                vz_of_t.data->pop_front();
            }
            r_of_t.auto_world->clear();
            v_of_t.auto_world->clear();
            if (r_of_t.view->visible)  r_of_t.auto_world->adjust(*r_of_t.data);
            if (x_of_t.view->visible)  r_of_t.auto_world->adjust(*x_of_t.data);
            if (y_of_t.view->visible)  r_of_t.auto_world->adjust(*y_of_t.data);
            if (z_of_t.view->visible)  r_of_t.auto_world->adjust(*z_of_t.data);
            if (v_of_t.view->visible)  v_of_t.auto_world->adjust(*v_of_t.data);
            if (vx_of_t.view->visible) v_of_t.auto_world->adjust(*vx_of_t.data);
            if (vy_of_t.view->visible) v_of_t.auto_world->adjust(*vy_of_t.data);
            if (vz_of_t.view->visible) v_of_t.auto_world->adjust(*vz_of_t.data);

            r_of_t.auto_world->flush();
            v_of_t.auto_world->flush();
            dlg.m_cXyzPlot.RedrawBuffer();
            dlg.m_cVxyzPlot.RedrawBuffer();
            dlg.Invoke([&] ()
            {
                t = _t;
                dlg.UpdateData(FALSE);
                dlg.m_cXyzPlot.SwapBuffers();
                dlg.m_cVxyzPlot.SwapBuffers();
                dlg.m_cXyzPlot.RedrawWindow();
                dlg.m_cVxyzPlot.RedrawWindow();
            });

            Sleep(16);
        }
        x_dx = rk4_solve(system, _t, _dt, x_dx.x, x_dx.dx);
        _t += _dt;
        ++iteration;
    }
    return 0;
}

CelectromagneticDlg::CelectromagneticDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CelectromagneticDlg::IDD, pParent)
    , m_pWorkerThread(NULL)
    , m_bWorking(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CelectromagneticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_XYZ_T, m_cXyzPlot);
    DDX_Control(pDX, IDC_VXYZ_T, m_cVxyzPlot);
    DDX_Text(pDX, IDC_EDIT1, v0.x);
    DDX_Text(pDX, IDC_EDIT2, v0.y);
    DDX_Text(pDX, IDC_EDIT3, v0.z);
    DDX_Text(pDX, IDC_EDIT4, e0.x);
    DDX_Text(pDX, IDC_EDIT5, e0.y);
    DDX_Text(pDX, IDC_EDIT6, e0.z);
    DDX_Text(pDX, IDC_EDIT7, b0.x);
    DDX_Text(pDX, IDC_EDIT8, b0.y);
    DDX_Text(pDX, IDC_EDIT9, b0.z);
    DDX_Text(pDX, IDC_EDIT10, dt);
    DDX_Text(pDX, IDC_EDIT11, t);
    DDX_Control(pDX, IDC_CHECK1, m_aVisibilityChecks[0]);
    DDX_Control(pDX, IDC_CHECK2, m_aVisibilityChecks[1]);
    DDX_Control(pDX, IDC_CHECK3, m_aVisibilityChecks[2]);
    DDX_Control(pDX, IDC_CHECK4, m_aVisibilityChecks[3]);
    DDX_Control(pDX, IDC_CHECK5, m_aVisibilityChecks[4]);
    DDX_Control(pDX, IDC_CHECK6, m_aVisibilityChecks[5]);
    DDX_Control(pDX, IDC_CHECK7, m_aVisibilityChecks[6]);
    DDX_Control(pDX, IDC_CHECK8, m_aVisibilityChecks[7]);
}

BEGIN_MESSAGE_MAP(CelectromagneticDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CelectromagneticDlg::OnBnClickedButton1)
    ON_MESSAGE(WM_INVOKE, &CelectromagneticDlg::OnInvoke)
    ON_BN_CLICKED(IDC_BUTTON2, &CelectromagneticDlg::OnBnClickedButton2)
    ON_COMMAND_RANGE(IDC_CHECK1, IDC_CHECK8, &CelectromagneticDlg::OnBnClickedVisibilityCheck)
END_MESSAGE_MAP()


// CelectromagneticDlg message handlers

BOOL CelectromagneticDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    m_aVisibilityChecks[0].SetCheck(TRUE);
    m_aVisibilityChecks[1].SetCheck(TRUE);
    m_aVisibilityChecks[2].SetCheck(TRUE);
    m_aVisibilityChecks[3].SetCheck(TRUE);
    m_aVisibilityChecks[4].SetCheck(TRUE);
    m_aVisibilityChecks[5].SetCheck(TRUE);
    m_aVisibilityChecks[6].SetCheck(TRUE);
    m_aVisibilityChecks[7].SetCheck(TRUE);

    auto_viewport_params params;
    params.factors = { 0, 0, 0.1, 0.1 };
    auto_viewport < points_t > ::ptr_t ravp = min_max_auto_viewport < points_t > ::create();
    auto_viewport < points_t > ::ptr_t vavp = min_max_auto_viewport < points_t > ::create();
    ravp->set_params(params);
    vavp->set_params(params);

    x_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(255, 0, 0), 3))
        .with_data()
        .with_auto_viewport(ravp);
    y_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(0, 180, 0), 3))
        .with_data()
        .with_auto_viewport(ravp);
    z_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(0, 180, 180), 3))
        .with_data()
        .with_auto_viewport(ravp);
    r_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(255, 255, 255), 3))
        .with_data()
        .with_auto_viewport(ravp);
    vx_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(255, 0, 0), 3))
        .with_data()
        .with_auto_viewport(vavp);
    vy_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(0, 180, 0), 3))
        .with_data()
        .with_auto_viewport(vavp);
    vz_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(0, 180, 180), 3))
        .with_data()
        .with_auto_viewport(vavp);
    v_of_t
        .with_view()
        .with_view_line_pen(plot::palette::pen(RGB(255, 150, 0), 3))
        .with_data()
        .with_auto_viewport(vavp);

    m_cXyzPlot.background = palette::brush();
    m_cVxyzPlot.background = palette::brush();
    m_cXyzPlot.triple_buffered = true;
    m_cVxyzPlot.triple_buffered = true;

    m_cXyzPlot.plot_layer.with(
        viewporter::create(
            tick_drawable::create(
                layer_drawable::create(
                    std::vector < drawable::ptr_t > ({
                        r_of_t.view, x_of_t.view, y_of_t.view, z_of_t.view
                    })
                ),
                const_n_tick_factory<axe::x>::create(
                    make_simple_tick_formatter(1),
                    0,
                    10
                ),
                const_n_tick_factory<axe::y>::create(
                    make_simple_tick_formatter(3),
                    0,
                    5
                ),
                palette::pen(RGB(80, 80, 80)),
                RGB(200, 200, 200)
            ),
            make_viewport_mapper(r_of_t.viewport_mapper)
        )
    );

    m_cVxyzPlot.plot_layer.with(
        viewporter::create(
            tick_drawable::create(
                layer_drawable::create(
                    std::vector < drawable::ptr_t > ({
                        v_of_t.view, vx_of_t.view, vy_of_t.view, vz_of_t.view
                    })
                ),
                const_n_tick_factory<axe::x>::create(
                    make_simple_tick_formatter(1),
                    0,
                    10
                ),
                const_n_tick_factory<axe::y>::create(
                    make_simple_tick_formatter(3),
                    0,
                    5
                ),
                palette::pen(RGB(80, 80, 80)),
                RGB(200, 200, 200)
            ),
            make_viewport_mapper(v_of_t.viewport_mapper)
        )
    );

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CelectromagneticDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CelectromagneticDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CelectromagneticDlg::OnBnClickedButton1()
{
    UpdateData(TRUE);

    StartSimulationThread();
}


BOOL CelectromagneticDlg::DestroyWindow()
{
    StopSimulationThread();

    return CDialogEx::DestroyWindow();
}


void CelectromagneticDlg::StartSimulationThread()
{
    if (this->m_bWorking)
    {
        return;
    }
    this->m_bWorking = TRUE;
    this->m_pWorkerThread = AfxBeginThread(&SimulationThreadProc, this, 0, 0, CREATE_SUSPENDED);
    this->m_pWorkerThread->m_bAutoDelete = FALSE;
    ResumeThread(this->m_pWorkerThread->m_hThread);
}


void CelectromagneticDlg::StopSimulationThread()
{
    if (this->m_bWorking)
    {
        this->m_bWorking = FALSE;
        while (MsgWaitForMultipleObjects(
            1, &this->m_pWorkerThread->m_hThread, FALSE, INFINITE, QS_SENDMESSAGE) != WAIT_OBJECT_0)
        {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        //this->m_pWorkerThread->Delete();
        delete this->m_pWorkerThread;
        this->m_pWorkerThread = NULL;
    }
}


void CelectromagneticDlg::Invoke(const std::function < void () > & fn)
{
    SendMessage(WM_INVOKE, 0, (LPARAM)&fn);
}


afx_msg LRESULT CelectromagneticDlg::OnInvoke(WPARAM wParam, LPARAM lParam)
{
    (*(const std::function < void () > *) lParam)();
    return 0;
}


void CelectromagneticDlg::OnBnClickedButton2()
{
    StopSimulationThread();
}


void CelectromagneticDlg::OnBnClickedVisibilityCheck(UINT nID)
{
    r_of_t. view->visible = (m_aVisibilityChecks[0].GetCheck() == TRUE);
    x_of_t. view->visible = (m_aVisibilityChecks[1].GetCheck() == TRUE);
    y_of_t. view->visible = (m_aVisibilityChecks[2].GetCheck() == TRUE);
    z_of_t. view->visible = (m_aVisibilityChecks[3].GetCheck() == TRUE);
    v_of_t. view->visible = (m_aVisibilityChecks[4].GetCheck() == TRUE);
    vx_of_t.view->visible = (m_aVisibilityChecks[5].GetCheck() == TRUE);
    vy_of_t.view->visible = (m_aVisibilityChecks[6].GetCheck() == TRUE);
    vz_of_t.view->visible = (m_aVisibilityChecks[7].GetCheck() == TRUE);
}
