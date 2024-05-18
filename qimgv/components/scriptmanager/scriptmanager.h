#pragma once

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMap>
#include <QDebug>
#include <QStringList>
#include <QProcess>
#include "utils/script.h"
#include "sourcecontainers/image.h"
#include "settings.h"

class ScriptManager : public QObject
{
    Q_OBJECT

  public:
    static ScriptManager *getInstance();
    ~ScriptManager() override;

    void runScript(QString const &scriptName, std::shared_ptr<Image> const &img);
    void readScripts();
    void saveScripts() const;
    void removeScript(QString const &scriptName);
    void addScript(QString const &scriptName, Script const &script);

    ND Script getScript(QString const &scriptName) const;
    ND bool   scriptExists(QString const &scriptName) const;

    static auto runCommand(QString const &cmd) -> QString;
    static void runCommandDetached(QString const &cmd);
    static auto allScripts() -> QMap<QString, Script> const &;
    static auto scriptNames() -> QList<QString>;

    ScriptManager(ScriptManager const &)                = delete;
    ScriptManager(ScriptManager &&) noexcept            = delete;
    ScriptManager &operator=(ScriptManager const &)     = delete;
    ScriptManager &operator=(ScriptManager &&) noexcept = delete;

  Q_SIGNALS:
    void error(QString);

  private:
    QMap<QString, Script> scripts; // <name, script>

    explicit    ScriptManager(QObject *parent = nullptr);
    static void processArguments(QStringList &cmd, std::shared_ptr<Image> const &img);
};

extern ScriptManager *scriptManager;
