#include "video-dock.hpp"

#include <QLabel>
#include <QTimer>

#include "obs-frontend-api.h"
#include "obs-module.h"
#include "util/config-file.h"
#include "util/lexer.h"

#define QT_UTF8(str) QString::fromUtf8(str)
#define QT_TO_UTF8(str) str.toUtf8().constData()

static void frontend_event(enum obs_frontend_event event, void *data)
{
	auto videoDock = static_cast<VideoDock *>(data);
	if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGED ||
	    event == OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED) {
		videoDock->UpdateValues();
	}
	const bool active = obs_frontend_streaming_active() ||
			    obs_frontend_recording_active() ||
			    obs_frontend_replay_buffer_active() ||
			    obs_frontend_virtualcam_active();
	if (active == videoDock->isEnabled()) {
		videoDock->setEnabled(!active);
	}
}

static void frontend_save_load(obs_data_t *save_data, bool saving, void *data)
{
	UNUSED_PARAMETER(save_data);
	auto videoDock = static_cast<VideoDock *>(data);
	if (saving) {

	} else {
		videoDock->UpdateValues();
	}
}

static inline bool ResTooHigh(uint32_t cx, uint32_t cy)
{
	return cx > 16384 || cy > 16384;
}

static inline bool ResTooLow(uint32_t cx, uint32_t cy)
{
	return cx < 8 || cy < 8;
}

struct BaseLexer {
	lexer lex;

public:
	inline BaseLexer() { lexer_init(&lex); }
	inline ~BaseLexer() { lexer_free(&lex); }
	operator lexer *() { return &lex; }
};

/* parses "[width]x[height]", string, i.e. 1024x768 */
static bool ConvertResText(const char *res, uint32_t &cx, uint32_t &cy)
{
	BaseLexer lex;
	base_token token;

	lexer_start(lex, res);

	/* parse width */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	cx = std::stoul(token.text.array);

	/* parse 'x' */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (strref_cmpi(&token.text, "x") != 0)
		return false;

	/* parse height */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	cy = std::stoul(token.text.array);

	/* shouldn't be any more tokens after this */
	//if (lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
	//	return false;

	if (ResTooHigh(cx, cy) || ResTooLow(cx, cy)) {
		cx = cy = 0;
		return false;
	}

	return true;
}

VideoDock::VideoDock(QWidget *parent) : QDockWidget(parent)
{
	setFeatures(DockWidgetMovable | DockWidgetFloatable);
	setWindowTitle(QT_UTF8(obs_module_text("VideoDock")));
	setObjectName("VideoDock");
	setFloating(true);
	hide();

	mainLayout = new QVBoxLayout(this);

	auto *baseLabel = new QLabel(this);
	baseLabel->setObjectName(QStringLiteral("baseLabel"));
	baseLabel->setText(QT_UTF8(obs_module_text("BaseResolution")));

	mainLayout->addWidget(baseLabel);

	baseResolution = new QComboBox(this);
	baseResolution->setObjectName(QStringLiteral("baseResolution"));
	baseResolution->setDuplicatesEnabled(false);
	baseResolution->setFrame(true);
	//baseResolution->setEditable(true);
	baseResolution->addItem(QStringLiteral("960x540 SD"));
	baseResolution->addItem(QStringLiteral("1280x720 HD"));
	baseResolution->addItem(QStringLiteral("1920x1080 Full HD"));
	baseResolution->addItem(QStringLiteral("3840x2160 4K UHD"));
	baseResolution->addItem(QStringLiteral("4096x2160 4K DCI"));

	baseLabel->setBuddy(baseResolution);

	mainLayout->addWidget(baseResolution);
	auto comboIndexChanged = static_cast<void (QComboBox::*)(int)>(
		&QComboBox::currentIndexChanged);
	connect(baseResolution, comboIndexChanged, [=](int index) {
		UNUSED_PARAMETER(index);
		uint32_t cx = 0, cy = 0;
		if (!ConvertResText(QT_TO_UTF8(baseResolution->currentText()),
				    cx, cy))
			return;
		if (baseX == cx && baseY == cy)
			return;
		auto *config = obs_frontend_get_profile_config();
		if (!config)
			return;
		baseX = cx;
		baseY = cy;
		config_set_uint(config, "Video", "BaseCX", cx);
		config_set_uint(config, "Video", "BaseCY", cy);
		config_save(config);
		obs_frontend_reset_video();
	});

	auto *outputLabel = new QLabel(this);
	outputLabel->setObjectName(QStringLiteral("outputLabel"));
	outputLabel->setText(QT_UTF8(obs_module_text("OutputResolution")));

	mainLayout->addWidget(outputLabel);

	outputResolution = new QComboBox(this);

	outputResolution->setObjectName(QStringLiteral("outputResolution"));
	outputResolution->setDuplicatesEnabled(false);
	outputResolution->setFrame(true);
	//outputResolution->setEditable(true);
	outputResolution->addItem(QStringLiteral("960x540 SD"));
	outputResolution->addItem(QStringLiteral("1280x720 HD"));
	outputResolution->addItem(QStringLiteral("1920x1080 Full HD"));
	outputResolution->addItem(QStringLiteral("3840x2160 4K UHD"));
	outputResolution->addItem(QStringLiteral("4096x2160 4K DCI"));

	outputLabel->setBuddy(outputResolution);

	mainLayout->addWidget(outputResolution);

	connect(outputResolution, comboIndexChanged, [=](int index) {
		UNUSED_PARAMETER(index);
		uint32_t cx = 0, cy = 0;
		if (!ConvertResText(QT_TO_UTF8(outputResolution->currentText()),
				    cx, cy))
			return;
		if (outputX == cx && outputY == cy)
			return;
		auto *config = obs_frontend_get_profile_config();
		if (!config)
			return;
		outputX = cx;
		outputY = cy;
		config_set_uint(config, "Video", "OutputCX", cx);
		config_set_uint(config, "Video", "OutputCY", cy);
		config_save(config);
		obs_frontend_reset_video();
	});

	auto *fpsLabel = new QLabel(this);
	fpsLabel->setObjectName(QStringLiteral("fpsLabel"));
	fpsLabel->setText(QT_UTF8(obs_module_text("FPS")));

	mainLayout->addWidget(fpsLabel);

	fps = new QComboBox(this);

	fps->setObjectName(QStringLiteral("fpsCommon"));
	fps->setDuplicatesEnabled(false);
	fps->setFrame(true);
#ifdef LOUPER
	fps->addItem(QStringLiteral("24"));
	fps->addItem(QStringLiteral("25"));
#else
	fps->addItem(QStringLiteral("10"));
	fps->addItem(QStringLiteral("20"));
	fps->addItem("24 NTSC");
	fps->addItem("25 PAL");
#endif
	fps->addItem(QStringLiteral("29.97"));
	fps->addItem(QStringLiteral("30"));
	fps->addItem(QStringLiteral("48"));
#ifdef LOUPER
	fps->addItem(QStringLiteral("50"));
#else
	fps->addItem("50 PAL");
#endif
	fps->addItem(QStringLiteral("59.94"));
	fps->addItem(QStringLiteral("60"));

	fpsLabel->setBuddy(fps);

	mainLayout->addWidget(fps);

	connect(fps, comboIndexChanged, [=](int index) {
		UNUSED_PARAMETER(index);
		double fpss = 0.0;
		sscanf(QT_TO_UTF8(fps->currentText()), "%lf", &fpss);
		if (fpss < 1.0)
			return;
		if (fpss == fpsd)
			return;

		auto *config = obs_frontend_get_profile_config();
		if (!config)
			return;
		fpsd = fpss;
		config_set_uint(config, "Video", "FPSType", 0); //FPSCommon
		config_set_string(config, "Video", "FPSCommon",
				  QT_TO_UTF8(fps->currentText()));
		config_save(config);
		obs_frontend_reset_video();
	});

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

VideoDock::~VideoDock()
{
	obs_frontend_remove_event_callback(frontend_event, this);
	obs_frontend_remove_save_callback(frontend_save_load, this);
}

void VideoDock::UpdateValues()
{
	auto *config = obs_frontend_get_profile_config();
	if (!config)
		return;
	if (config_get_uint(config, "Video", "BaseCX") != baseX ||
	    config_get_uint(config, "Video", "BaseCY") != baseY) {
		baseX = config_get_uint(config, "Video", "BaseCX");
		baseY = config_get_uint(config, "Video", "BaseCY");
		QString res = "";
		res += QString::number(baseX);
		res += "x";
		res += QString::number(baseY);
		baseResolution->setCurrentIndex(
			baseResolution->findText(res, Qt::MatchStartsWith));
	}
	if (config_get_uint(config, "Video", "OutputCX") != outputX ||
	    config_get_uint(config, "Video", "OutputCY") != outputY) {
		outputX = config_get_uint(config, "Video", "OutputCX");
		outputY = config_get_uint(config, "Video", "OutputCY");
		QString res = "";
		res += QString::number(outputX);
		res += "x";
		res += QString::number(outputY);
		outputResolution->setCurrentIndex(
			outputResolution->findText(res, Qt::MatchStartsWith));
	}
	const char *fpsc = config_get_string(config, "Video", "FPSCommon");
	double fpss = 0.0;
	if (sscanf(fpsc, "%lf", &fpss) && fpss >= 1.0 && fpss != fpsd) {
		fpsd = fpss;

		int closest_fps_index = -1;
		double closest_diff = 1000000000000.0;
		for (int i = 0; i < fps->count(); i++) {
			double com_fpsd;
			sscanf(QT_TO_UTF8(fps->itemText(i)), "%lf", &com_fpsd);
			double diff = fabs(com_fpsd - fpsd);
			if (diff < closest_diff) {
				closest_diff = diff;
				closest_fps_index = i;
			}
		}
		fps->setCurrentIndex(closest_fps_index);
	}
}
