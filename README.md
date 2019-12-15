# Usage example

```Python
import win32clipboard
import winwire

def print_clipboard_contents():
    std_fmts = {
        1: "CF_TEXT",
        2: "CF_BITMAP",
        3: "CF_METAFILEPICT",
        4: "CF_SYLK",
        5: "CF_DIF",
        6: "CF_TIFF",
        7: "CF_OEMTEXT",
        8: "CF_DIB",
        9: "CF_PALETTE",
        10: "CF_PENDATA",
        11: "CF_RIFF",
        12: "CF_WAVE",
        13: "CF_UNICODETEXT",
        14: "CF_ENHMETAFILE",
        15: "CF_HDROP",
        16: "CF_LOCALE",
        17: "CF_DIBV5",
        0x81: "CF_DSPTEXT",
        0x82: "CF_DSPBITMAP",
        0x83: "CF_DSPMETAFILEPICT",
        0x8E: "CF_DSPENHMETAFILE"
    }
    
    def fmt_name(fmt):
        name = std_fmts.get(fmt, None)
        
        if name is not None:
            return name
        
        try:
            return win32clipboard.GetClipboardFormatName(fmt)
        except:
            return format(fmt, "#04x")
    
    try:
        try:
            hwnd = win32clipboard.GetClipboardOwner()
            print(f"Clipboard owner: {hwnd:#04x}")
        
        except:
            print(f"Clipboard owner: <unknown>")
        
        fmt = win32clipboard.EnumClipboardFormats(0)
        
        while fmt:
            try:
                data = win32clipboard.GetClipboardData(fmt)
                print(f"Clipboard format: {fmt_name(fmt)}\n{str(data)}\n")
            
            except:
                print(f"Clipboard format: {fmt_name(fmt)}\n<not printable>\n")
            
            fmt = win32clipboard.EnumClipboardFormats(fmt)
        
    except Exception as err:
        print(str(err))

winwire.listen_clipboard("test", print_clipboard_contents)
```

```Python
import win32clipboard
import winwire

def print_clipboard_text():
    if win32clipboard.IsClipboardFormatAvailable(13):
        print(win32clipboard.GetClipboardData(13))
    elif win32clipboard.IsClipboardFormatAvailable(1):
        print(win32clipboard.GetClipboardData(1).decode('ascii'))

winwire.listen_clipboard("test", print_clipboard_text)
```
