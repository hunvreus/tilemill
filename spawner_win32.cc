#include <windows.h>
#include <winbase.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h> 
#include <Shlobj.h> // for getting user path
#include <string> 

#define BUFSIZE 4096 
 
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
HANDLE g_hInputFile = NULL;

void ErrorExit(LPTSTR lpszFunction, DWORD dw) 
{ 
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("TileMill Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

void ErrorExit(LPTSTR lpszFunction)
{
    ErrorExit(lpszFunction,GetLastError());
}

bool writeToLog(const char* chBuf)
{
    DWORD dwRead = strlen(chBuf); 
    DWORD dwWritten(0); 
    BOOL bSuccess = FALSE;

    return WriteFile(g_hInputFile, chBuf, 
                           dwRead, &dwWritten, NULL);

}

void ReadFromPipe(void) 

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 
   DWORD dwRead, dwWritten; 
   CHAR chBuf[BUFSIZE]; 
   BOOL bSuccess = FALSE;
   HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

// Close the write end of the pipe before reading from the 
// read end of the pipe, to control child process execution.
// The pipe is assumed to have enough buffer space to hold the
// data the child process has already written to it.
 
   if (!CloseHandle(g_hChildStd_OUT_Wr)) 
      ErrorExit(TEXT("StdOutWr CloseHandle")); 
 
   for (;;) 
   { 
      bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
	  if( ! bSuccess ) break;
      std::string debug_line(chBuf);
	  std::string substring = debug_line.substr(0,static_cast<size_t>(dwRead));
	  substring += "\nPlease report this to https://github.com/mapbox/tilemill/issues\n";
	  if (substring.find("Error:") !=std::string::npos)
	  {
		  MessageBox(NULL, static_cast<LPCSTR>(substring.c_str()), TEXT("TileMill Error"), MB_OK);
		  ExitProcess(1);
	  }
	  bSuccess = WriteFile(g_hInputFile, chBuf, 
                           dwRead, &dwWritten, NULL);
      if (! bSuccess ) break;
      	  
   }
}

void CreateChildProcess()
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
   TCHAR szCmdline[]=TEXT("node.exe index.js");
   PROCESS_INFORMATION piProcInfo; 
   STARTUPINFO siStartInfo;
   BOOL bSuccess = FALSE; 
 
// Set up members of the PROCESS_INFORMATION structure. 
 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = g_hChildStd_OUT_Wr;
   siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
   siStartInfo.hStdInput = g_hChildStd_IN_Rd;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
// Create the child process. 
    
   bSuccess = CreateProcess(NULL, 
      szCmdline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      CREATE_NO_WINDOW,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   
   // If an error occurs, exit the application. 
   if ( ! bSuccess ) 
      ErrorExit(TEXT("CreateProcess"));
   else 
   {
      // Close handles to the child process and its primary thread.
      // Some applications might keep these handles to monitor the status
      // of the child process, for example. 

      CloseHandle(piProcInfo.hProcess);
      CloseHandle(piProcInfo.hThread);
   }
}

/*
void CreateChildProcess2()
{

 STARTUPINFOA startup;
  ZeroMemory( &startup, sizeof(STARTUPINFOA) );
  PROCESS_INFORMATION pi;
  ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );

  startup.cb = sizeof(startup);
  startup.lpReserved = NULL;
  startup.lpDesktop = NULL;
  startup.lpTitle = NULL;
  startup.dwFlags = 0;
  //startup.dwFlags = STARTF_USESHOWWINDOW;
  //startup.wShowWindow = SW_HIDE;
  startup.cbReserved2 = 0;
  startup.lpReserved2 = NULL;
  
  if (!CreateProcessA(
    NULL,
    "node.exe index.js",
    NULL,
    NULL,
    FALSE,
    CREATE_NO_WINDOW,
    NULL,
    NULL,
    &startup,
    &pi))
  {
     //OutputDebugString("did not start node");
     ErrorExit("TileMill.exe could not start node:",GetLastError());  
  }
  else
  {
    // Resume the external process thread.
    DWORD resumeThreadResult = ResumeThread(pi.hThread);
    // ResumeThread() returns 1 which is OK
    // (it means that the thread was suspended but then restarted)

    // Wait for the external process to finish.
    DWORD waitForSingelObjectResult =  WaitForSingleObject(pi.hProcess, INFINITE);
    // WaitForSingleObject() returns 0 which is OK.

    // Get the exit code of the external process.
    DWORD exitCode;
    if(!GetExitCodeProcess(pi.hProcess, &exitCode))
    {
        // Handle error.
		ErrorExit("TileMill.exe subprocess...",exitCode);
    }
    else
    {
        // There is no error but exitCode is 128, a value that
        // doesn't exist in the external process (and even if it
        // existed it doesn't matter as it isn't being invoked any more)
        // Error code 128 is ERROR_WAIT_NO_CHILDREN which would make some
        // sense *if* GetExitCodeProcess() returned FALSE and then I were to
        // get ERROR_WAIT_NO_CHILDREN with GetLastError()
		//ErrorExit("TileMill.exe subprocess2...",GetLastError());
    }

    // PROCESS_INFORMATION handles for process and thread are closed.
  }
}
*/

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow) {
   SECURITY_ATTRIBUTES saAttr; 
 
// Set the bInheritHandle flag so pipe handles are inherited. 
 
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   saAttr.bInheritHandle = TRUE; 
   saAttr.lpSecurityDescriptor = NULL; 

// Create a pipe for the child process's STDOUT. 
 
   if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
      ErrorExit(TEXT("StdoutRd CreatePipe")); 

// Ensure the read handle to the pipe for STDOUT is not inherited.

   if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdout SetHandleInformation")); 

// Create a pipe for the child process's STDIN. 
 
   if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
      ErrorExit(TEXT("Stdin CreatePipe")); 

// Ensure the write handle to the pipe for STDIN is not inherited. 
 
   if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdin SetHandleInformation")); 

   /* 
   * Set env variable in the current process.  It'll get inherited by
   * node process.
   */
  if (!SetEnvironmentVariableA("PROJ_LIB","data\\proj\\nad"))
      ErrorExit("TileMill.exe setting env: ",GetLastError());
  if (!SetEnvironmentVariableA("GDAL_DATA","data\\gdal\\data"))
      ErrorExit("TileMill.exe setting env: ",GetLastError());
  if (!SetEnvironmentVariableA("PATH","node_modules\\mapnik\\lib\\mapnik\\lib;node_modules\\zipfile\\lib;%PATH%"))
      ErrorExit("TileMill.exe setting env: ",GetLastError());

  // Create the child process. 
  CreateChildProcess();

  
   // String buffer for holding the path.

   TCHAR strPath[ MAX_PATH ];

// Get the special folder path.
SHGetSpecialFolderPath(
    0,       // Hwnd
    strPath, // String buffer.
    CSIDL_PROFILE, // CSLID of folder
    FALSE ); // Create if doesn't exists?
	
   std::string logpath(strPath);
   logpath += "\\tilemill.log";
   g_hInputFile = CreateFile(
       logpath.c_str(), 
       GENERIC_WRITE, 
       FILE_SHARE_READ, 
       NULL, 
       CREATE_ALWAYS, 
       FILE_ATTRIBUTE_NORMAL, 
       NULL); 

    if ( g_hInputFile == INVALID_HANDLE_VALUE ) 
      ErrorExit(TEXT("CreateFile")); 
  
   // Read from pipe that is the standard output for child process. 
   writeToLog("Starting TileMill...\n");
  
   ReadFromPipe(); 

  // The remaining open handles are cleaned up when this process terminates. 
  // To avoid resource leaks in a larger application, close handles explicitly. 

   return 0;

 
}
