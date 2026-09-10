#pragma once

#include <QObject>
#include <QString>

class QComboBox;

enum class ThemeMode {
    System = 0,
    Light = 1,
    Dark = 2
};

class ThemeManager : public QObject {
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);

    void setMode(ThemeMode mode);
    [[nodiscard]] ThemeMode mode() const;
    [[nodiscard]] bool isDark() const;
    void apply();
    void populateCombo(QComboBox* combo) const;

signals:
    void themeChanged(bool dark);

private:
    [[nodiscard]] bool resolveDark() const;
    void loadStyleSheet(bool dark);

    ThemeMode m_mode = ThemeMode::System;
};
