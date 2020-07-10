// Windows specific functions for the Vector Mame Menu

#include "vmmstddef.h"
#include "WinVMM.h"
#include <stdio.h>
#include <stdlib.h>

int LEDstate=0;

/******************************************************************
	Write to keyboard LEDs value held in global variable LEDstate
 - only if state has changed
*******************************************************************/
void setLEDs(int leds)
{
   if (LEDstate != leds)
   {
//
//		Todo...
//		Code needed - anyone care to contrubute a Win32 routine
//		to enable you to set the Caps/NumLock/scrollLock LEDs?
//
//		Suff I've googled but not had time to play with yet is below...
//
      LEDstate = leds;
   }
}

/*
#include <windows.h>
#define err if (GetLastError() != 0) return GetLastError();

extern "C" __declspec(dllexport) int __stdcall TurnLed(int state)
{
    DWORD tmp = 4;
    DWORD buf = (2*GetKeyState(VK_NUMLOCK) + 4*GetKeyState(VK_CAPITAL) + state)<<16;
    DefineDosDevice(DDD_RAW_TARGET_PATH, "Kbd", "\\Device\\KeyboardClass1"); err
    HANDLE kbd = CreateFile("\\\\.\\Kbd", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL,  NULL); err
    DeviceIoControl(kbd, CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS), &buf, sizeof(buf), 0, 0, &tmp, 0); err
    DefineDosDevice(DDD_REMOVE_DEFINITION, "Kbd", 0); err
    CloseHandle(kbd); err
    return 0;
}




#include <windows.h>
#include <stdbool.h>
#include <winioctl.h>
#include <assert.h>
#include <stdint.h>
#include <signal.h>
#include "keyboard.h"

#define IOCTL_KEYBOARD_SET_INDICATORS        CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC       CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)


static HANDLE kbd;


void OpenKeyboardDevice()
{
  if(!DefineDosDevice(DDD_RAW_TARGET_PATH, "Kbd000000",
		      "\\Device\\KeyboardClass0"))
  {
    assert(false);
  }

  kbd = CreateFile("\\\\.\\Kbd000000", GENERIC_WRITE, 0,
		   NULL, OPEN_EXISTING, 0, NULL);
  assert(kbd);
}


void CloseKeyboardDevice()
{
  DefineDosDevice(DDD_REMOVE_DEFINITION, "Kbd000000", NULL);
  CloseHandle(kbd);
}



int toggle_led(bool toggle, int led)
{
  uint32_t input = 0, output = 0;

  DWORD len;
  if(!DeviceIoControl(kbd, IOCTL_KEYBOARD_QUERY_INDICATORS,
		      &input, sizeof(input),
		      &output, sizeof(output),
		      &len, NULL))
    return GetLastError();

  input = output;
  input &= ~(led << 16);
  if(toggle)
    input |= led << 16;

  if(!DeviceIoControl(kbd, IOCTL_KEYBOARD_SET_INDICATORS,
		      &input, sizeof(input),
		      NULL, 0,
		      &len, NULL))
    return GetLastError();
  return 0;
}






#include <windows.h>
#include <winioctl.h>
#include <dinput.h>
 
#pragma comment(linker, "/subsystem:windows")
 
// most of these are taken from ntkbdddk.h or w/e it's called
#define IOCTL_KEYBOARD_SET_INDICATORS CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)
 
#define KBD_CAPS 4
#define KBD_NUM 2
#define KBD_SCROLL 1
 
#define KBD_ON 0x80
#define KBD_OFF 0x40
 
typedef struct _KEYBOARD_INDICATOR_PARAMETERS 
{
    WORD wId;
    WORD wFlags;
} KEYBOARD_INDICATOR_PARAMETERS, *PKEYBOARD_INDICATOR_PARAMETERS;
 
typedef struct {
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
    DWORD time;
    DWORD * dwExtraInfo;
} KBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;
 
void KeybdLight(DWORD wFlags);
void Flash();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
 
int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, 
                   int nShowCmd)
{
    MSG msg;
 
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Flash, 0, 0, 0);
    SetWindowsHookEx(13, LowLevelKeyboardProc, GetModuleHandle(0), 0);
 
    while(GetMessage(&msg, 0, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
void KeybdLight(DWORD wFlags)
{
    HANDLE hKeybd;
    KEYBOARD_INDICATOR_PARAMETERS buffer;
    DWORD retlen;
     
    DefineDosDevice(DDD_RAW_TARGET_PATH, "Keybd", "\\Device\\KeyboardClass0");
    hKeybd = CreateFile("\\\\.\\Keybd", GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
     
    DeviceIoControl(hKeybd, IOCTL_KEYBOARD_QUERY_INDICATORS, 0, 0, &buffer, sizeof(buffer), &retlen, 0);
 
    if(wFlags & KBD_ON)
        buffer.wFlags |= wFlags & 15;
    else if(wFlags & KBD_OFF)
        buffer.wFlags =~ (WORD)wFlags & 15;
    else
        buffer.wFlags ^= wFlags & 15;
     
    DeviceIoControl(hKeybd, IOCTL_KEYBOARD_SET_INDICATORS, &buffer, sizeof(buffer), 0, 0, &retlen, 0);
     
    DefineDosDevice(DDD_REMOVE_DEFINITION, "Keybd", 0);
    CloseHandle(hKeybd);
}
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT * kbdllhookstruct = (KBDLLHOOKSTRUCT *)lParam;
 
    if(kbdllhookstruct->scanCode == DIK_ESCAPE)
    {
            KeybdLight(KBD_CAPS | KBD_NUM | KBD_SCROLL | KBD_OFF);
            PostQuitMessage(0);
    }
    return 0;
}
void Flash()
{
    for(;;)
    {   
        KeybdLight(KBD_NUM | KBD_SCROLL);
        Sleep(1000);
        KeybdLight(KBD_NUM | KBD_SCROLL);
        KeybdLight(KBD_CAPS);
        Sleep(1000);
        KeybdLight(KBD_CAPS);
    }
}

*/


