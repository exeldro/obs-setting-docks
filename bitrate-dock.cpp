#include "bitrate-dock.hpp"

#include <QLabel>
#include <QTimer>

#include "obs-frontend-api.h"
#include "obs-module.h"
#include "util/config-file.h"
#include "util/dstr.h"

#define QT_UTF8(str) QString::fromUtf8(str)
#define QT_TO_UTF8(str) str.toUtf8().constData()

static void frontend_save_load(obs_data_t *save_data, bool saving, void *data)
{
	auto bitrateDock = static_cast<BitrateDock *>(data);
	if (saving) {

	} else {
		bitrateDock->UpdateValues();
	}
}

BitrateDock::BitrateDock(QWidget *parent) : QDockWidget(parent)
{
	setFeatures(DockWidgetMovable | DockWidgetFloatable);
	setWindowTitle(QT_UTF8(obs_module_text("BitrateDock")));
	setObjectName("BitrateDock");
	setFloating(true);
	hide();

	mainLayout = new QVBoxLayout(this);

	auto *vBitrateLabel = new QLabel(this);
	vBitrateLabel->setObjectName(QStringLiteral("vBitrateLabel"));
	vBitrateLabel->setText(QT_UTF8(obs_module_text("VideoBitrate")));

	mainLayout->addWidget(vBitrateLabel);
#ifdef LOUPER
	vBitrateEdit = new QComboBox(this);
	vBitrateEdit->addItem(QStringLiteral("2500 Kbps ") +
			      QT_UTF8(obs_module_text("Balanced")));
	vBitrateEdit->addItem(QStringLiteral("4000 Kbps ") +
			      QT_UTF8(obs_module_text("High")));
	vBitrateEdit->addItem(QStringLiteral("6000 Kbps ") +
			      QT_UTF8(obs_module_text("VeryHigh")));
	vBitrateEdit->addItem(QT_UTF8(obs_module_text("ShowMore")));
	connect(vBitrateEdit, &QComboBox::currentTextChanged, [=](QString text) {
		int index = -1;
		if (text == QT_UTF8(obs_module_text("ShowMore")) &&
		    (index = vBitrateEdit->findText(
			     QT_UTF8(obs_module_text("ShowMore")))) != -1) {

			vBitrateEdit->removeItem(index);
			vBitrateEdit->addItem(
				QStringLiteral("8000 Kbps ") +
				QT_UTF8(obs_module_text("SuperHigh")));
			vBitrateEdit->addItem(
				QStringLiteral("12000 Kbps ") +
				QT_UTF8(obs_module_text("UltraHigh")));
			vBitrateEdit->addItem(
				QStringLiteral("15000 Kbps ") +
				QT_UTF8(obs_module_text("Extreme")));
			vBitrateEdit->addItem(
				QT_UTF8(obs_module_text("CustomValue")));
			return;
		}
		if (text == QT_UTF8(obs_module_text("CustomValue")) &&
		    (index = vBitrateEdit->findText(
			     QT_UTF8(obs_module_text("CustomValue")))) != -1) {
			vBitrateEdit->removeItem(index);
			vBitrateEdit->setEditable(true);
			return;
		}
		index = vBitrateEdit->findText(text);
		if (index > -1 && vBitrateEdit->isEditable()) {
			vBitrateEdit->setEditable(false);
			vBitrateEdit->addItem(
				QT_UTF8(obs_module_text("CustomValue")));
		}
		uint64_t bitrate = 0;
		sscanf(QT_TO_UTF8(text), "%u", &bitrate);
		if (bitrate == 0 || vBitrate == bitrate)
			return;
		vBitrate = bitrate;
#else
	vBitrateEdit = new QSpinBox(this);
	vBitrateEdit->setObjectName(QStringLiteral("vBitrateEdit"));
	vBitrateEdit->setMinimum(200);
	vBitrateEdit->setMaximum(1000000);
	vBitrateEdit->setValue(2000);
	vBitrateEdit->setSuffix(" Kbps");

	auto sbValueChanged =
		static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged);
	connect(vBitrateEdit, sbValueChanged, [=](int value) {
		if (value == vBitrate)
			return;
		vBitrate = value;
#endif
		auto *config = obs_frontend_get_profile_config();
		if (!config)
			return;
		config_set_uint(config, "SimpleOutput", "VBitrate", vBitrate);
		config_set_uint(config, "AdvOut", "FFVBitrate", vBitrate);
		config_save(config);

		auto *output = obs_get_output_by_name("simple_stream");
		if (!output)
			output = obs_get_output_by_name("adv_stream");
		if (!output)
			return;
		auto *vencoder = obs_output_get_video_encoder(output);
		if (vencoder) {
			auto *vencoder_settings = obs_data_create();
			obs_data_set_int(vencoder_settings, "bitrate",
					 vBitrate);
			obs_encoder_update(vencoder, vencoder_settings);
			obs_data_release(vencoder_settings);
		}
		obs_output_release(output);
	});
	vBitrateLabel->setBuddy(vBitrateEdit);
	mainLayout->addWidget(vBitrateEdit);

	auto *aBitrateLabel = new QLabel(this);
	aBitrateLabel->setObjectName(QStringLiteral("aBitrateLabel"));
	aBitrateLabel->setText(QT_UTF8(obs_module_text("AudioBitrate")));

	mainLayout->addWidget(aBitrateLabel);

	aBitrateEdit = new QComboBox(this);
#ifdef LOUPER
	aBitrateEdit->addItem(QStringLiteral("128 Kbps ") +
			      QT_UTF8(obs_module_text("Medium")));
	aBitrateEdit->addItem(QStringLiteral("160 Kbps ") +
			      QT_UTF8(obs_module_text("High")));
	aBitrateEdit->addItem(QStringLiteral("320 Kbps ") +
			      QT_UTF8(obs_module_text("SuperHigh")));
	aBitrateEdit->addItem(QStringLiteral("512 Kbps ") +
			      QT_UTF8(obs_module_text("Extreme")));
#else
	aBitrateEdit->addItem(QStringLiteral("48"));
	aBitrateEdit->addItem(QStringLiteral("64"));
	aBitrateEdit->addItem(QStringLiteral("80"));
	aBitrateEdit->addItem(QStringLiteral("96"));
	aBitrateEdit->addItem(QStringLiteral("112"));
	aBitrateEdit->addItem(QStringLiteral("128"));
	aBitrateEdit->addItem(QStringLiteral("160"));
	aBitrateEdit->addItem(QStringLiteral("192"));
	aBitrateEdit->addItem(QStringLiteral("224"));
	aBitrateEdit->addItem(QStringLiteral("256"));
	aBitrateEdit->addItem(QStringLiteral("288"));
	aBitrateEdit->addItem(QStringLiteral("320"));
	aBitrateEdit->addItem(QStringLiteral("352"));
#endif
	auto comboIndexChanged = static_cast<void (QComboBox::*)(int)>(
		&QComboBox::currentIndexChanged);
	connect(aBitrateEdit, comboIndexChanged, [=](int index) {
		uint64_t bitrate = 0;
		sscanf(QT_TO_UTF8(aBitrateEdit->currentText()), "%u", &bitrate);
		if (bitrate == 0 || aBitrate == bitrate)
			return;
		aBitrate = bitrate;
		auto *config = obs_frontend_get_profile_config();
		if (!config)
			return;
		config_set_uint(config, "SimpleOutput", "ABitrate", aBitrate);
		config_set_uint(config, "AdvOut", "FFABitrate", aBitrate);
		config_set_uint(config, "AdvOut", "Track1Bitrate", aBitrate);
		config_set_uint(config, "AdvOut", "Track2Bitrate", aBitrate);
		config_set_uint(config, "AdvOut", "Track3Bitrate", aBitrate);
		config_set_uint(config, "AdvOut", "Track4Bitrate", aBitrate);
		config_set_uint(config, "AdvOut", "Track5Bitrate", aBitrate);
		config_set_uint(config, "AdvOut", "Track6Bitrate", aBitrate);
		config_save(config);
		auto *output = obs_get_output_by_name("simple_stream");
		if (!output)
			output = obs_get_output_by_name("adv_stream");
		if (!output)
			return;
		int track = 0;
		auto *aencoder_settings = obs_data_create();
		obs_data_set_int(aencoder_settings, "bitrate", aBitrate);
		for (;;) {
			auto *aencoder =
				obs_output_get_audio_encoder(output, track);
			if (!aencoder)
				break;
			obs_encoder_update(aencoder, aencoder_settings);
			track++;
		}
		obs_data_release(aencoder_settings);
		obs_output_release(output);
	});

	aBitrateLabel->setBuddy(aBitrateEdit);
	mainLayout->addWidget(aBitrateEdit);

	auto *verticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum,
					       QSizePolicy::Expanding);

	mainLayout->addItem(verticalSpacer);

	auto *dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);

	setWidget(dockWidgetContents);

	auto *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [=]() { UpdateValues(); });
	timer->start(1000);

	obs_frontend_add_save_callback(frontend_save_load, this);
}

BitrateDock::~BitrateDock()
{
	obs_frontend_remove_save_callback(frontend_save_load, this);
}

void BitrateDock::UpdateValues()
{
	auto *config = obs_frontend_get_profile_config();
	if (!config)
		return;

	uint64_t videoBitrate;
	uint64_t audioBitrate;
	const char *mode = config_get_string(config, "Output", "Mode");
	if (astrcmpi(mode, "Advanced") == 0) {
		videoBitrate = config_get_uint(config, "AdvOut", "FFVBitrate");
		audioBitrate = config_get_uint(config, "AdvOut", "FFABitrate");
		auto track = config_get_int(config, "AdvOut", "TrackIndex");
		if (track == 6) {
			audioBitrate = config_get_uint(config, "AdvOut",
						       "Track6Bitrate");
		} else if (track == 5) {
			audioBitrate = config_get_uint(config, "AdvOut",
						       "Track5Bitrate");
		} else if (track == 4) {
			audioBitrate = config_get_uint(config, "AdvOut",
						       "Track4Bitrate");
		} else if (track == 3) {
			audioBitrate = config_get_uint(config, "AdvOut",
						       "Track3Bitrate");
		} else if (track == 2) {
			audioBitrate = config_get_uint(config, "AdvOut",
						       "Track2Bitrate");
		} else {
			audioBitrate = config_get_uint(config, "AdvOut",
						       "Track1Bitrate");
		}
	} else {
		videoBitrate =
			config_get_uint(config, "SimpleOutput", "VBitrate");
		audioBitrate =
			config_get_uint(config, "SimpleOutput", "ABitrate");
	}
	if (videoBitrate != vBitrate) {

		vBitrate = videoBitrate;
#ifdef LOUPER
		int index = vBitrateEdit->findText(QString::number(vBitrate),
						   Qt::MatchStartsWith);
		if (index != -1) {
			vBitrateEdit->setCurrentIndex(index);
			if (vBitrateEdit->isEditable()) {
				vBitrateEdit->setEditable(false);
				vBitrateEdit->addItem(QT_UTF8(
					obs_module_text("CustomValue")));
			}
		} else {
			index = vBitrateEdit->findText(
				QT_UTF8(obs_module_text("ShowMore")));
			if (index != -1) {
				vBitrateEdit->removeItem(index);
				vBitrateEdit->addItem(
					QStringLiteral("8000 Kbps ") +
					QT_UTF8(obs_module_text("SuperHigh")));
				vBitrateEdit->addItem(
					QStringLiteral("12000 Kbps ") +
					QT_UTF8(obs_module_text("UltraHigh")));
				vBitrateEdit->addItem(
					QStringLiteral("15000 Kbps ") +
					QT_UTF8(obs_module_text("Extreme")));
				index = vBitrateEdit->findText(
					QString::number(vBitrate),
					Qt::MatchStartsWith);
				if (index != -1) {
					vBitrateEdit->setCurrentIndex(index);
					vBitrateEdit->setEditable(false);
					vBitrateEdit->addItem(
						QT_UTF8(obs_module_text(
							"CustomValue")));
				}
			}
			if (index == -1) {
				vBitrateEdit->setEditable(true);
				vBitrateEdit->setCurrentText(
					QString::number(vBitrate));
			}
		}
#else
		vBitrateEdit->setValue(vBitrate);
#endif
	}
	if (audioBitrate != aBitrate) {
		aBitrate = audioBitrate;
		const int index = aBitrateEdit->findText(
			QString::number(aBitrate), Qt::MatchStartsWith);
		if (index != -1)
			aBitrateEdit->setCurrentIndex(index);
		else
			aBitrateEdit->setCurrentText(QString::number(aBitrate));
	}
}
