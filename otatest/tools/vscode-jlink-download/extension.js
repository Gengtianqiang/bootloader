const vscode = require('vscode');
const fs = require('node:fs');

const buttons = [
    {
        name: 'Bootloader',
        marker: '.vscode/flash-bootloader.jlink',
        command: 'localEmbedded.downloadBootloader',
        task: 'J-Link: Download Bootloader',
        text: '$(cloud-download) 下载 Bootloader',
        tooltip: '构建并用 J-Link Commander 烧录 Bootloader'
    },
    {
        name: 'App',
        marker: '.vscode/flash-otatest.jlink',
        command: 'localEmbedded.downloadOtatest',
        task: 'J-Link: Download App',
        text: '$(cloud-download) 下载 App',
        tooltip: '构建并用 J-Link Commander 烧录 App'
    },
    {
        name: 'OTA Package',
        marker: 'tools/Build-Step2Package.ps1',
        command: 'localEmbedded.buildOtatestPackage',
        task: 'OTA: Build Encrypted Package',
        text: '$(package) 生成 OTA 包',
        tooltip: '构建 App 并从 ELF 生成 Step2 加密升级包'
    },
];

function matchingFolders(marker) {
    return (vscode.workspace.workspaceFolders ?? []).filter(folder =>
        folder.uri.scheme === 'file' &&
        fs.existsSync(vscode.Uri.joinPath(folder.uri, ...marker.split('/')).fsPath)
    );
}

function activate(context) {
    for (const entry of buttons) {
        const button = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 55);
        button.text = entry.text;
        button.tooltip = entry.tooltip;
        button.command = entry.command;

        function updateButton() {
            if (matchingFolders(entry.marker).length) {
                button.show();
            } else {
                button.hide();
            }
        }

        context.subscriptions.push(
            button,
            vscode.workspace.onDidChangeWorkspaceFolders(updateButton),
            vscode.commands.registerCommand(entry.command, async () => {
                if (matchingFolders(entry.marker).length !== 1) {
                    await vscode.window.showWarningMessage('请打开包含该任务的单个工程。');
                    return;
                }
                try {
                    await vscode.commands.executeCommand('workbench.action.tasks.runTask', entry.task);
                } catch (error) {
                    await vscode.window.showErrorMessage(`无法启动 ${entry.name} 任务。 ${error.message}`);
                }
            })
        );
        updateButton();
    }
}

module.exports = { activate };
