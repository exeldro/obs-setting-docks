#include <QDockWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>

#include "obs-frontend-api.h"

class BitrateDock : public QDockWidget {
	Q_OBJECT

private:
	QVBoxLayout *mainLayout;
	QSpinBox *vBitrateEdit;
	QComboBox *aBitrateEdit;
	uint64_t vBitrate;
	uint64_t aBitrate;
private slots:

public:
	BitrateDock(QWidget *parent = nullptr);
	~BitrateDock();
	void UpdateValues();
};
