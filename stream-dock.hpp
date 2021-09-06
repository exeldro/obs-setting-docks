

#include <QDockWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "obs-frontend-api.h"

class StreamDock : public QDockWidget {
	Q_OBJECT

private:
	QVBoxLayout *mainLayout;
	QLineEdit *serverEdit;
	QLineEdit *keyEdit;
	QPushButton *showButton;
	QString key;
	QString server;
private slots:


public:
	StreamDock(QWidget *parent = nullptr);
	~StreamDock();
	void UpdateValues();
};
