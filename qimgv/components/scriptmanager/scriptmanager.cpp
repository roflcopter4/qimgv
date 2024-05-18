#include "scriptmanager.h"

ScriptManager *scriptManager = nullptr;

ScriptManager::ScriptManager(QObject *parent) : QObject(parent)
{}

ScriptManager::~ScriptManager()
{
    scriptManager->saveScripts();
    delete scriptManager;
}

ScriptManager *ScriptManager::getInstance()
{
    if(!scriptManager) {
        scriptManager = new ScriptManager();
        scriptManager->readScripts();
    }
    return scriptManager;
}

void ScriptManager::runScript(QString const &scriptName, std::shared_ptr<Image> const &img)
{
    if(scripts.contains(scriptName)) {
        Script script = scripts.value(scriptName);
        if(script.command.isEmpty())
            return;
        QProcess exec(this);

        //auto arguments = splitCommandLine(script.command);
        auto arguments = QProcess::splitCommand(script.command);
        processArguments(arguments, img);
        QString program = arguments.takeAt(0);

        qDebug() << QSV("Running program") << program << QSV("with arguments") << arguments;
        if(script.blocking) {
            exec.start(program, arguments);
            if(!exec.waitForStarted())
                qDebug() << QSV("Unable not run application/script \"") << program << QSV("\". Make sure it is an executable.");
            exec.waitForFinished(10000);
        } else {
            if(!QProcess::startDetached(program, arguments)) {
                QFileInfo fi(program);
                QString errorString;
                if(fi.isFile() && !fi.isExecutable())
                    errorString = QS("Error:  ") + program + QS("  is not an executable.");
                else
                    errorString = QS("Error: unable run application/script. See README for working examples.");
                emit error(errorString);
                qWarning() << errorString;
            }
        }
    } else {
        qDebug() << QSV("[ScriptManager] File ") << scriptName << QSV(" does not exist.");
    }
}

QString ScriptManager::runCommand(QString const &cmd)
{
    QProcess exec;
    QStringList cmdSplit = QProcess::splitCommand(cmd);
    exec.start(cmdSplit.takeAt(0), cmdSplit);
    exec.waitForFinished(2000);
    return exec.readAllStandardOutput();
}

void ScriptManager::runCommandDetached(QString const &cmd)
{
    QStringList cmdSplit = QProcess::splitCommand(cmd);
    QProcess::startDetached(cmdSplit.takeAt(0), cmdSplit);
}

void ScriptManager::processArguments(QStringList &cmd, std::shared_ptr<Image> const &img)
{
    static constexpr auto file = QSV("%file%");
    for (auto &i : cmd)
        if (i == file)
            i = img->filePath();
}

bool ScriptManager::scriptExists(QString const &scriptName) const
{
    return scripts.contains(scriptName);
}

void ScriptManager::readScripts()
{
    settings->readScripts(scripts);
}

void ScriptManager::saveScripts() const
{
    settings->saveScripts(scripts);
}

// replaces if it already exists
void ScriptManager::addScript(QString const &scriptName, Script const &script)
{
    if (scripts.contains(scriptName)) {
        qDebug() << QSV("[ScriptManager] Replacing script") << scriptName;
        scripts.remove(scriptName);
    }
    scripts.insert(scriptName, script);
}

void ScriptManager::removeScript(QString const &scriptName)
{
    scripts.remove(scriptName);
}

QMap<QString, Script> const &ScriptManager::allScripts()
{
    return scriptManager->scripts;
}

QList<QString> ScriptManager::scriptNames()
{
    return scriptManager->scripts.keys();
}

Script ScriptManager::getScript(QString const &scriptName) const
{
    return scripts.value(scriptName);
}
