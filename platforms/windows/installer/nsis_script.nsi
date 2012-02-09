; TileMill nsis installer script

!define PRODUCT_NAME "TileMill"
!define PRODUCT_VERSION "0.9.0"
!define PRODUCT_PUBLISHER "MapBox"
!define PRODUCT_WEB_SITE "http://mapbox.com/tilemill/docs"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

; firewall extras
!addplugindir "nsisFirewall"
!include "FileFunc.nsh"

RequestExecutionLevel admin

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\tilemill.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "TileMill"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"

; Pages
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "TileMill-${PRODUCT_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES\TileMill"

Section "TileMill (required)"
  SectionIn RO
  SetOutPath "$INSTDIR"
  SetOverwrite try

  ;; Base Installation
  File /r /x *Recycle.Bin* /x installer /x demo /x *.git \
      /x *.git* /x Makefil* /x test /x *.vcx* /x *.ipch \
	  /x ipch /x AppData /x deps /x include /x expresso \
	  /x osx /x ubuntu /x virtualbox /x *.idx /x *.pack \
	  /x *.sln /x *.sdf /x *.lib /x Default \
	  ..\..\..\..\tilemill\*.*
  ExecWait "$INSTDIR\platforms\windows\vcredist_x86.exe /q /norestart"

SectionEnd

Section "Start Menu Shortcuts"
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\TileMill"
  CreateShortCut "$SMPROGRAMS\TileMill\Start TileMill.lnk" "$INSTDIR\TileMill.exe" "" \
      "$INSTDIR\platforms\windows\tilemill.ico" "" \
	  SW_SHOWNORMAL \
      ALT|CONTROL|t "TileMill"
	  
  CreateShortCut "$SMPROGRAMS\TileMill\Uninstall TileMill.lnk" "$INSTDIR\Uninstall-TileMill.exe"
SectionEnd

; Add firewall rule
Section "Allow TileMill port access in firewall (highly recommended)"
	; Add TileMill to the authorized list
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\node.exe" "Evented I/O for V8 JavaScript"
	Pop $0
	IntCmp $0 0 +3
		MessageBox MB_OK "A problem happened while adding Node.exe (used by TileMill) to the Firewall exception list (result=$0)"
		Return
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\Uninstall-TileMill.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall-TileMill.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section "Uninstall"
   SetShellVarContext all
   ; Remove Node.js from the authorized list
   nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\node.exe"
   Pop $0
   IntCmp $0 0 +3
       MessageBox MB_OK "A problem happened while removing Node.exe (used by TileMill) from the Firewall exception list (result=$0)"
       Return

  Delete "$INSTDIR\Uninstall-TileMill.exe"
  RMDir /r "$INSTDIR\*.*"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\TileMill\*.*"
  RMDir "$SMPROGRAMS\TileMill"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd