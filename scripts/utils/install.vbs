Option Explicit

' Script de instalacao para Mega_Emu usando VBScript para elevacao de privilegios

' Funcao para mostrar mensagem de debug
Sub ShowDebug(msg)
    WScript.Echo "[DEBUG] " & msg
End Sub

' Funcao para obter o caminho do script
Function GetScriptPath()
    GetScriptPath = CreateObject("Scripting.FileSystemObject").GetParentFolderName(WScript.ScriptFullName)
End Function

' Funcao para executar comando com privilegios elevados
Sub RunElevated(cmd)
    On Error Resume Next

    Dim shell, shellApp, fso
    Set fso = CreateObject("Scripting.FileSystemObject")

    ShowDebug "Criando objetos Shell..."
    Set shell = CreateObject("WScript.Shell")
    Set shellApp = CreateObject("Shell.Application")

    If Err.Number <> 0 Then
        MsgBox "Erro ao criar objetos Shell: " & Err.Description, 16, "Erro"
        WScript.Quit 1
    End If

    ShowDebug "Preparando comando PowerShell..."
    ShowDebug "Comando: " & cmd

    ' Primeiro, executar o script de correção do perfil
    Dim fixProfileScript
    fixProfileScript = fso.BuildPath(GetScriptPath(), "fix_profile.ps1")

    If fso.FileExists(fixProfileScript) Then
        ShowDebug "Corrigindo perfil do PowerShell..."
        shell.Run "powershell -NoProfile -ExecutionPolicy Bypass -File """ & fixProfileScript & """", 0, True
    End If

    ' Executar PowerShell com privilegios elevados
    shellApp.ShellExecute "powershell.exe", _
        "-NoProfile -ExecutionPolicy Bypass -Command """ & cmd & """ -Verbose", _
        "", "runas", 1

    If Err.Number <> 0 Then
        MsgBox "Erro ao executar PowerShell: " & Err.Description, 16, "Erro"
        WScript.Quit 1
    End If

    Set shellApp = Nothing
    Set shell = Nothing
    Set fso = Nothing
End Sub

' Funcao principal
Sub Main()
    On Error Resume Next

    Dim scriptPath, powershellScript, cmd, fso
    Set fso = CreateObject("Scripting.FileSystemObject")

    ShowDebug "Iniciando processo de instalacao..."

    ' Obter caminho do script PowerShell
    scriptPath = GetScriptPath()
    powershellScript = fso.BuildPath(scriptPath, "install.ps1")

    ShowDebug "Caminho do script: " & powershellScript

    ' Verificar se o script existe
    If Not fso.FileExists(powershellScript) Then
        MsgBox "ERRO: Script PowerShell nao encontrado em:" & vbCrLf & powershellScript, 16, "Erro de Instalacao"
        WScript.Quit 1
    End If

    ShowDebug "Script PowerShell encontrado"

    ' Preparar comando PowerShell com ambiente limpo
    cmd = "$env:POWERSHELL_SKIP_PROFILE = $true; " & _
          "Set-Location '" & scriptPath & "'; " & _
          "Write-Host '[VBS] Diretorio atual:' (Get-Location).Path; " & _
          "Write-Host '[VBS] Executando script:' '" & powershellScript & "'; " & _
          "& '" & powershellScript & "'"

    ShowDebug "Executando com privilegios elevados..."
    RunElevated cmd

    If Err.Number <> 0 Then
        MsgBox "Erro durante a execucao: " & Err.Description, 16, "Erro"
        WScript.Quit 1
    End If
End Sub

' Executar script principal
Main
