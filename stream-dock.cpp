#include "stream-dock.hpp"

#include <QLabel>
#include <QTimer>

#include "obs-frontend-api.h"
#include "obs-module.h"

#define QT_UTF8(str) QString::fromUtf8(str)
#define QT_TO_UTF8(str) str.toUtf8().constData()

static void frontend_event(enum obs_frontend_event event, void *data)
{
	auto streamDock = static_cast<StreamDock *>(data);
	if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGED ||
	    event == OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED) {
		streamDock->UpdateValues();
	} else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTING ||
		   event == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
		streamDock->setEnabled(false);
	} else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPING ||
		   event == OBS_FRONTEND_EVENT_STREAMING_STOPPED) {
		streamDock->setEnabled(true);
	}
}

void frontend_save_load(obs_data_t *save_data, bool saving, void *data)
{
	auto streamDock = static_cast<StreamDock *>(data);
	if (saving) {

	} else {
		streamDock->UpdateValues();
	}
}

StreamDock::StreamDock(QWidget *parent) : QDockWidget(parent)
{
	setFeatures(DockWidgetMovable | DockWidgetFloatable);
	setWindowTitle(QT_UTF8(obs_module_text("StreamDock")));
	setObjectName("StreamDock");
	setFloating(true);
	hide();

	mainLayout = new QVBoxLayout(this);

	auto *serverLabel = new QLabel(this);
	serverLabel->setObjectName(QStringLiteral("serverLabel"));
	serverLabel->setText(QT_UTF8(obs_module_text("Server")));

	mainLayout->addWidget(serverLabel);

	serverEdit = new QLineEdit(this);
	serverEdit->setObjectName(QStringLiteral("server"));
	//server->setInputMask(QStringLiteral(""));
	serverEdit->setText(QStringLiteral(""));

	connect(serverEdit, &QLineEdit::textChanged, [=]() {
		if (server == serverEdit->text())
			return;
		server = serverEdit->text();
		auto *service = obs_frontend_get_streaming_service();
		if (!service)
			return;
		obs_data_t *settings = obs_service_get_settings(service);
		obs_data_set_string(settings, "server", QT_TO_UTF8(server));
		obs_service_update(service, settings);
		obs_data_release(settings);
		obs_frontend_save_streaming_service();
	});

	mainLayout->addWidget(serverEdit);

	auto *keyLabel = new QLabel(this);
	keyLabel->setObjectName(QStringLiteral("keyLabel"));
	keyLabel->setText(QT_UTF8(obs_module_text("StreamKey")));

	mainLayout->addWidget(keyLabel);

	auto *horizontalLayout = new QHBoxLayout(this);

	keyEdit = new QLineEdit(this);
	keyEdit->setObjectName(QStringLiteral("key"));
	keyEdit->setInputMask(QStringLiteral(""));
	keyEdit->setText(QStringLiteral(""));
	keyEdit->setEchoMode(QLineEdit::Password);
	connect(keyEdit, &QLineEdit::textChanged, [=]() {
		if (key == keyEdit->text())
			return;
		key = keyEdit->text();
		auto *service = obs_frontend_get_streaming_service();
		if (!service)
			return;
		obs_data_t *settings = obs_service_get_settings(service);
		obs_data_set_string(settings, "key", QT_TO_UTF8(key));
		obs_service_update(service, settings);
		obs_data_release(settings);
		obs_frontend_save_streaming_service();
	});

	horizontalLayout->addWidget(keyEdit);

	showButton = new QPushButton(this);
	showButton->setObjectName(QStringLiteral("show"));
	showButton->setText(QT_UTF8(obs_module_text("Show")));

	connect(showButton, &QPushButton::clicked, [=]() {
		if (keyEdit->echoMode() == QLineEdit::Password) {
			keyEdit->setEchoMode(QLineEdit::Normal);
			showButton->setText(QT_UTF8(obs_module_text("Hide")));
		} else {
			keyEdit->setEchoMode(QLineEdit::Password);
			showButton->setText(QT_UTF8(obs_module_text("Show")));
		}
	});

	horizontalLayout->addWidget(showButton);

	mainLayout->addLayout(horizontalLayout);

	auto *verticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum,
					       QSizePolicy::Expanding);

	mainLayout->addItem(verticalSpacer);

	auto *dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);

	setWidget(dockWidgetContents);

	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [=]() { UpdateValues(); });
	timer->start(1000);

	obs_frontend_add_event_callback(frontend_event, this);
	obs_frontend_add_save_callback(frontend_save_load, this);
}

StreamDock::~StreamDock()
{
	obs_frontend_remove_event_callback(frontend_event, this);
	obs_frontend_remove_save_callback(frontend_save_load, this);
}

void StreamDock::UpdateValues()
{
	auto *service = obs_frontend_get_streaming_service();
	if (!service)
		return;

	auto s = QT_UTF8(obs_service_get_url(service));
	if (server != s) {
		server = s;
		serverEdit->setText(s);
	}
	auto k = QT_UTF8(obs_service_get_key(service));
	if (k != key) {
		key = k;
		keyEdit->setText(k);
	}
}
