#define WINVER 0x0A00
#define PY_SSIZE_T_CLEAN
#include "winwire.h"
#include "uo_linkpool.h"
#include <windows.h>

static BOOL is_init = FALSE;
static HANDLE thrd;
static HWND hWnd;
static PyGILState_STATE gstate;

typedef struct ww_clipcb
{
    char *name;
    PyObject *cb;
} ww_clipcb;

uo_decl_linklist(ww_clipcb, ww_clipcb);
uo_impl_linklist(ww_clipcb, ww_clipcb);
uo_decl_linkpool(ww_clipcb, ww_clipcb);
uo_impl_linkpool(ww_clipcb, ww_clipcb);

static uo_linklist clipcb_list;

LRESULT CALLBACK WindowProc(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_CLIPBOARDUPDATE:
            if (!OpenClipboard(hWnd))
                break;

            gstate = PyGILState_Ensure();
            ww_clipcb_linklist *link = (ww_clipcb_linklist *)clipcb_list.next;

            while (&clipcb_list != &link->link)
            {
                PyObject *arglist = Py_BuildValue("()");
                PyObject *res = PyEval_CallObject(link->item.cb, arglist);
                Py_DECREF(arglist);

                if (PyObject_IsTrue(res))
                    break;

                link = ww_clipcb_linklist_next(link);
            }

            PyGILState_Release(gstate);
            CloseClipboard();
            return 0;

        default:
            //printf("uMsg: 0x%x\n", uMsg);
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static DWORD WINAPI winwire_run(
    LPVOID lpParam)
{
    ww_clipcb_linkpool_thrd_init();
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

    // Create message-only window
    static const char* class_name = "winwire";
    WNDCLASSEX wx = {
        .cbSize = sizeof(WNDCLASSEX),
        .lpfnWndProc = WindowProc,
        .hInstance = hInstance,
        .lpszClassName = class_name
    };

    if (!RegisterClassEx(&wx))
        return 1;

    hWnd = CreateWindowEx(0, class_name, "winwire", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

    // Monitor clipboard
    BOOL pass = AddClipboardFormatListener(hWnd);

    // Run the message loop.
    if (pass)
    {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Shutdown
    RemoveClipboardFormatListener(hWnd);
    DestroyWindow(hWnd);
    UnregisterClass(class_name, hInstance);
    ww_clipcb_linkpool_thrd_quit();
}

static void winwire_init(void)
{
    if (is_init)
        return;

    ww_clipcb_linkpool_init();
    ww_clipcb_linkpool_thrd_init();
    uo_linklist_selflink(&clipcb_list);
    thrd = CreateThread(NULL, 0, winwire_run, NULL, 0, NULL);
    is_init = TRUE;
}

static void winwire_shutdown(
    void *arg)
{
    if (!is_init)
        return;

    SendMessage(hWnd, WM_CLOSE, 0, 0);

    while (!uo_linklist_is_empty(&clipcb_list))
    {
        ww_clipcb_linklist *link = (ww_clipcb_linklist *)clipcb_list.next;
        uo_linklist_unlink(&link->link);
        free(link->item.name);
        Py_DECREF(link->item.cb);
        ww_clipcb_linkpool_return(link);
    }

    ww_clipcb_linkpool_thrd_quit();

    is_init = FALSE;
}

static PyObject *listen_clipboard(
    PyObject *self, 
    PyObject *args)
{
    char *name;
    PyObject *cb;
    char *after = NULL;

    if (!PyArg_ParseTuple(args, "sO|s", &name, &cb, &after) || !PyCallable_Check(cb)) 
    {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    ww_clipcb_linklist *pos = (ww_clipcb_linklist *)clipcb_list.prev;

    if (after != NULL)
    {
        while (&clipcb_list != &pos->link)
        {
            if (strcmp(pos->item.name, name) == 0)
            {
                after = NULL;
                break;
            }

            pos = ww_clipcb_linklist_prev(pos);
        }

        if (after != NULL)
        {
            PyErr_SetString(PyExc_TypeError, "invalid 'after' argument");
            return NULL;
        }
    }

    ww_clipcb_linklist *link = ww_clipcb_linkpool_rent();
    Py_INCREF(cb);
    link->item.name = strdup(name);
    link->item.cb = cb;
    uo_linklist_link(pos->link.next, &link->link);

    Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
    { "listen_clipboard", listen_clipboard, METH_VARARGS, "Register a callback function for WM_CLIPBOARDUPDATE event." },
    { NULL, NULL, 0, NULL }
};

static PyModuleDef module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "winwire",
    .m_doc = "Module for interacting with Windows Messages and WinEvents using callbacks.",
    .m_size = -1,
    .m_methods = methods,
    .m_free = winwire_shutdown
};

PyObject *PyInit_winwire()
{
    winwire_init();
    return PyModule_Create(&module);
}
