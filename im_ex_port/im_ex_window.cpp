#include "im_ex_window.h"

ImExWindow::ImExWindow(QWidget* parent) : QTabWidget(parent)
{
    initLayout();

    setMinimumSize(300, 400);
}

void ImExWindow::initLayout()
{
    // 导出
    export_page = new ExportPage(this);
    addTab(export_page, QIcon(QPixmap(":/icons/export")), "导出");

    // 导入
    import_page = new ImportPage(this);
    addTab(import_page, QIcon(QPixmap(":/icons/import")), "导入");

    // 从码字风云导入
    import_mzfy_page = new ImportMzfyPage(this);
    addTab(import_mzfy_page,  QIcon(QPixmap(":/icons/mzfy")), "从码字风云导入");
}

void ImExWindow::toShow(int kind, QString def_name)
{
    current_novel_name = def_name;
    if (kind>0 && kind<=count())
    {
        setCurrentIndex(kind-1);
    }
    import_page->setNovelName(def_name);
    adjustSize();
    show();
}
