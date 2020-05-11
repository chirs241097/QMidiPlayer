#ifndef QMPPIANOWIDGET_HPP
#define QMPPIANOWIDGET_HPP

#include <QWidget>
#include <bitset>

class qmpPianoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit qmpPianoWidget(QWidget *parent = nullptr);
    void setKeyState(int key, bool state);
    void reset();
    QSize minimumSizeHint()const override;
    bool hasHeightForWidth()const override;
    int heightForWidth(int w)const override;

protected:
    void paintEvent(QPaintEvent *event)override;

private:
    bool keystates[128];
    QRectF getKeyRect(int key);
    void paintKey(QRectF keyrect, QColor keycolor);
};

#endif // QMPPIANOWIDGET_HPP
