#ifndef IM_EX_WINDOW
#define IM_EX_WINDOW

#include <QWidget>
#include <QObject>
#include <QTabWidget>
#include "globalvar.h"
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"
#include "exportpage.h"
#include "importpage.h"
#include "importmzfypage.h"

class ImExWindow : public QTabWidget
{
    Q_OBJECT
public:
    ImExWindow(QWidget* parent);
    void initLayout();
    void toShow(int kind, QString def_name);

signals:
    void signalImportFinished(QString novel_name);
    void signalImportMzfyFinished(QStringList novel_name_list);

public slots:

public:
    ImportPage* import_page;
    ExportPage* export_page;
    ImportMzfyPage* import_mzfy_page;

private:
    QString current_novel_name;
};

#endif
