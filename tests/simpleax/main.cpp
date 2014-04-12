//#include "SimpleAx.h"
//#include <QApplication>

//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);
//    SimpleAx w;
//    w.show();

//    return a.exec();
//}

#include <QAxBindable>
#include <QAxFactory>
#include <QApplication>
#include <QLayout>
#include <QSlider>
#include <QLCDNumber>
#include <QLineEdit>
#include <QMessageBox>

//! [0]
class MySimpleAX : public QWidget, public QAxBindable
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int value READ value WRITE setValue )
public:
    MySimpleAX(QWidget *parent = 0)
        : QWidget(parent)
    {
        QVBoxLayout *vbox = new QVBoxLayout( this );

        slider = new QSlider( Qt::Horizontal, this );
        LCD = new QLCDNumber( 3, this );
        edit = new QLineEdit( this );

        connect( slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)) );
        connect( edit, SIGNAL(textChanged(QString)), this, SLOT(setText(QString)) );

        vbox->addWidget( slider );
        vbox->addWidget( LCD );
        vbox->addWidget( edit );
    }

    QString text() const
    {
        return edit->text();
    }
    int value() const
    {
        return slider->value();
    }

signals:
    void someSignal();
    void valueChanged(int);
    void textChanged(const QString&);

public slots:
    void setText( const QString &string )
    {
        if ( !requestPropertyChange( "text" ) )
            return;

        edit->blockSignals( true );
        edit->setText( string );
        edit->blockSignals( false );
        emit someSignal();
        emit textChanged( string );

        propertyChanged( "text" );
    }
    void about()
    {
        emit someSignal();

        QMessageBox::information( this, "About QSimpleAX", "This is a Qt widget, and this slot has been\n"
                                  "called through ActiveX/OLE automation!" );
    }
    void setValue( int i )
    {
        if ( !requestPropertyChange( "value" ) )
            return;
        slider->blockSignals( true );
        slider->setValue( i );
        slider->blockSignals( false );
        LCD->display( i );
        emit valueChanged( i );

        propertyChanged( "value" );
    }

private:
    QSlider *slider;
    QLCDNumber *LCD;
    QLineEdit *edit;
};

//! [0]
#include "main.moc"

//! [1]
QAXFACTORY_DEFAULT(MySimpleAX,
                   "{EBF9056F-2E44-4460-93C9-0837E28AB5D8}",
                   "{1DAD4C11-28BD-4B10-8C21-3443AC8C786D}",
                   "{26EBD19D-D0E2-48CE-805B-898A4177A178}",
                   "{93D8DA36-BE1F-4F8D-999B-4ABC8C3B6472}",
                   "{C15D34A0-24D5-48EC-9395-285889AA20E0}")
//! [1]
