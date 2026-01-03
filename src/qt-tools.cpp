#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>

#include <QAction>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>

static QAction *g_tools_action = nullptr;

void register_qt_tools()
{
	if (g_tools_action) {
		return;
	}

	auto *main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	if (!main_window) {
		obs_log(LOG_WARNING, "[tint_filter] obs_frontend_get_main_window() returned nullptr");
		return;
	}

	g_tools_action = static_cast<QAction *>(obs_frontend_add_tools_menu_qaction(obs_module_text("TintToolsMenu")));
	if (!g_tools_action) {
		obs_log(LOG_WARNING, "[tint_filter] obs_frontend_add_tools_menu_qaction() returned nullptr");
		return;
	}

	QObject::connect(g_tools_action, &QAction::triggered, main_window, [main_window]() {
		QMessageBox::information(main_window, obs_module_text("TintToolsTitle"),
					 obs_module_text("TintToolsBody"));
	});
}

void unregister_qt_tools()
{
	g_tools_action = nullptr;
}