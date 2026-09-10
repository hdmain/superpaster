#include "theme/ThemeManager.h"

#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QIODevice>
#include <QPalette>
#include <QStyleHints>

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme) {
        if (m_mode == ThemeMode::System) {
            apply();
        }
    });
#endif
}

void ThemeManager::setMode(ThemeMode mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    apply();
}

ThemeMode ThemeManager::mode() const
{
    return m_mode;
}

bool ThemeManager::isDark() const
{
    return resolveDark();
}

void ThemeManager::apply()
{
    const bool dark = resolveDark();
    loadStyleSheet(dark);
    emit themeChanged(dark);
}

void ThemeManager::populateCombo(QComboBox* combo) const
{
    combo->clear();
    combo->addItem(tr("System"), static_cast<int>(ThemeMode::System));
    combo->addItem(tr("Light"), static_cast<int>(ThemeMode::Light));
    combo->addItem(tr("Dark"), static_cast<int>(ThemeMode::Dark));
    combo->setCurrentIndex(static_cast<int>(m_mode));
}

bool ThemeManager::resolveDark() const
{
    switch (m_mode) {
    case ThemeMode::Light:
        return false;
    case ThemeMode::Dark:
        return true;
    case ThemeMode::System:
    default:
        break;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#else
    const QPalette pal = qApp->palette();
    return pal.color(QPalette::Window).lightness() < 128;
#endif
}

void ThemeManager::loadStyleSheet(bool dark)
{
    const QString path = dark ? QStringLiteral(":/styles/dark.qss")
                              : QStringLiteral(":/styles/light.qss");
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
    }
}
