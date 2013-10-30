; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=RMAExpress
AppVerName=RMAExpress 1.0.5 Release
AppPublisher=RMAExpress
AppPublisherURL=http://rmaexpress.bmbolstad.com
AppSupportURL=http://rmaexpress.bmbolstad.com
AppUpdatesURL=http://rmaexpress.bmbolstad.com
DefaultDirName={pf}\RMAExpress
DefaultGroupName=RMAExpress
WizardImageStretch=no
WizardImageFile="\\Bmbbox\tmp\RMAExpress\RMAExpress_MasterLOGO_Installer.bmp"
WizardSmallImageFile="\\Bmbbox\tmp\RMAExpress\RMAExpress_MasterLOGOSmall.bmp"
WizardImageBackColor=clWhite
AppCopyright=Copyright (C) 2003-2009 B. M. Bolstad.
BackSolid=yes

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Files]
Source: "\\Bmbbox\tmp\RMAExpress\RMAExpress.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "\\Bmbbox\tmp\RMAExpress\RMADataConv.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "\\Bmbbox\tmp\RMAExpress\RMAExpressConsole.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "\\Bmbbox\tmp\RMAExpress\RMAExpress_UsersGuide.pdf";  DestDir: "{app}"; Flags: ignoreversion
Source: "\\Bmbbox\tmp\RMAExpress\vcredist_x86.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\RMAExpress"; Filename: "{app}\RMAExpress.exe"
Name: "{group}\RMADataConv"; Filename: "{app}\RMADataConv.exe"
Name: "{group}\RMAExpress Users Guide"; Filename: "{app}\RMAExpress_UsersGuide.pdf"
Name: "{userdesktop}\RMAExpress"; Filename: "{app}\RMAExpress.exe"; Tasks: desktopicon
Name: "{userdesktop}\RMADataConv"; Filename: "{app}\RMADataConv.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\vcredist_x86.exe";  Parameters: "/q:a"; Flags:
Filename: "{app}\RMAExpress.exe"; Description: "Launch RMAExpress"; Flags: nowait postinstall skipifsilent
