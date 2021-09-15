

#include <QDockWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

#include "obs-frontend-api.h"

class VideoDock : public QDockWidget {
	Q_OBJECT

private:
	QVBoxLayout *mainLayout;
	QComboBox *baseResolution;
	QComboBox *outputResolution;
	QComboBox *fps;

	uint32_t baseX = 0;
	uint32_t baseY = 0;
	uint32_t outputX = 0;
	uint32_t outputY = 0;
	double fpsd;
private slots:

public:
	VideoDock(QWidget *parent = nullptr);
	~VideoDock();
	void UpdateValues();
};
