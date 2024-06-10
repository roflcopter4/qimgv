#include "ScriptManager.h"

ScriptManager *scriptManager = nullptr;

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
{}

ScriptManager::~ScriptManager()
{
    saveScripts();
}

ScriptManager *ScriptManager::getInstance()
{
    if(!scriptManager) {
        scriptManager = new ScriptManager();
        scriptManager->readScripts();
    }
    return scriptManager;
}

void ScriptManager::runScript(QString const &scriptName, QSharedPointer<Image> const &img)
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

        qDebug() << u"Running program" << program << u"with arguments" << arguments;
        if(script.blocking) {
            exec.start(program, arguments);
            if(!exec.waitForStarted())
                qDebug() << u"Unable not run application/script \"" << program << u"\". Make sure it is an executable.";
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
        qDebug() << u"[ScriptManager] File " << scriptName << u" does not exist.";
    }
}

QString ScriptManager::runCommand(QString const &cmd)
{
    QProcess exec;
    QStringList cmdSplit = QProcess::splitCommand(cmd);
    exec.start(cmdSplit.takeAt(0), cmdSplit);
    exec.waitForFinished(2000);
    return QString::fromUtf8(exec.readAllStandardOutput());
}

void ScriptManager::runCommandDetached(QString const &cmd)
{
    QStringList cmdSplit = QProcess::splitCommand(cmd);
    QProcess::startDetached(cmdSplit.takeAt(0), cmdSplit);
}

// TODO: what if filename contains one of the tags?
void ScriptManager::processArguments(QStringList &cmd, QSharedPointer<Image> const &img)
{
    QString field = QS("%file%");
    for (auto &i : cmd) {
        if(i.contains(field))
            i.replace(field, img->filePath());
#if defined _WIN32 && false
        // force "\" as a directory separator
        i.replace("/", "\\");
        i.replace("\\\\", "\\");
#endif
    }
}

// thanks stackoverflow
QStringList ScriptManager::splitCommandLine(QString const &cmdLine)
{
    QStringList list;
    QString arg;
    bool escape = false;
    enum { Idle, Arg, QuotedArg } state = Idle;
    foreach (QChar const c, cmdLine) {
        //if(!escape && c == '\\') {
        //    escape = true;
        //    continue;
        //}
        switch (state) {
        case Idle:
            if(!escape && c == u'"')
                state = QuotedArg;
            else if (escape || !c.isSpace()) {
                arg += c;
                state = Arg;
            }
            break;
        case Arg:
            if(!escape && c == u'"')
                state = QuotedArg;
            else if(escape || !c.isSpace())
                arg += c;
            else {
                list << arg;
                arg.clear();
                state = Idle;
            }
            break;
        case QuotedArg:
            if(!escape && c == u'"')
                state = arg.isEmpty() ? Idle : Arg;
            else
                arg += c;
            break;
        }
        escape = false;
    }
    if(!arg.isEmpty())
        list << arg;
    return list;
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
        qDebug() << u"[ScriptManager] Replacing script" << scriptName;
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
