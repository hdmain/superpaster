#include "ui/SettingsWindow.h"
#include "theme/ThemeManager.h"
#include "clipboard/ClipboardHistory.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSize>
#include <QSvgWidget>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(ThemeManager* themes, ClipboardHistory* history, QWidget* parent)
    : QWidget(parent)
    , m_themes(themes)
    , m_history(history)
{
    setObjectName(QStringLiteral("SettingsRoot"));
    setWindowTitle(QStringLiteral("Spaste"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/clipboard.svg")));
    setFixedSize(440, 420);
    buildUi();
}

void SettingsWindow::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("SettingsPanel"));
    panel->setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(16);

    auto* header = new QHBoxLayout();
    auto* logo = new QSvgWidget(QStringLiteral(":/icons/clipboard.svg"), panel);
    logo->setFixedSize(32, 32);
    auto* brand = new QLabel(QStringLiteral("Spaste"), panel);
    brand->setObjectName(QStringLiteral("BrandTitle"));
    header->addWidget(logo);
    header->addWidget(brand);
    header->addStretch();
    layout->addLayout(header);

    auto* intro = new QLabel(
        tr("Clipboard history stays in RAM only and is cleared when Spaste exits."),
        panel);
    intro->setObjectName(QStringLiteral("HintLabel"));
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto* appearanceTitle = new QLabel(tr("Appearance"), panel);
    appearanceTitle->setObjectName(QStringLiteral("SectionTitle"));
    layout->addWidget(appearanceTitle);

    auto* themeRow = new QHBoxLayout();
    auto* themeIcon = new QSvgWidget(QStringLiteral(":/icons/monitor.svg"), panel);
    themeIcon->setFixedSize(18, 18);
    auto* themeLabel = new QLabel(tr("Theme"), panel);
    themeLabel->setObjectName(QStringLiteral("HintLabel"));
    m_themeCombo = new QComboBox(panel);
    m_themes->populateCombo(m_themeCombo);
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        const auto mode = static_cast<ThemeMode>(m_themeCombo->itemData(index).toInt());
        m_themes->setMode(mode);
    });
    themeRow->addWidget(themeIcon);
    themeRow->addWidget(themeLabel);
    themeRow->addStretch();
    themeRow->addWidget(m_themeCombo, 1);
    layout->addLayout(themeRow);

    auto* shortcutsTitle = new QLabel(tr("Shortcuts"), panel);
    shortcutsTitle->setObjectName(QStringLiteral("SectionTitle"));
    layout->addWidget(shortcutsTitle);

    auto* shortcutHint = new QLabel(
        tr("Super + V — open clipboard overlay\n"
           "Launch Spaste — open this settings window"),
        panel);
    shortcutHint->setObjectName(QStringLiteral("HintLabel"));
    layout->addWidget(shortcutHint);

    auto* historyTitle = new QLabel(tr("History"), panel);
    historyTitle->setObjectName(QStringLiteral("SectionTitle"));
    layout->addWidget(historyTitle);

    auto* historyRow = new QHBoxLayout();
    auto* countLabel = new QLabel(panel);
    countLabel->setObjectName(QStringLiteral("HintLabel"));
    auto updateCount = [this, countLabel]() {
        countLabel->setText(tr("%1 / %2 items in memory")
                                .arg(m_history->count())
                                .arg(ClipboardHistory::kMaxItems));
    };
    updateCount();
    connect(m_history, &ClipboardHistory::historyChanged, this, updateCount);

    auto* clearBtn = new QPushButton(tr("Clear history"), panel);
    clearBtn->setIcon(QIcon(QStringLiteral(":/icons/trash.svg")));
    clearBtn->setIconSize(QSize(14, 14));
    connect(clearBtn, &QPushButton::clicked, m_history, &ClipboardHistory::clear);

    historyRow->addWidget(countLabel);
    historyRow->addStretch();
    historyRow->addWidget(clearBtn);
    layout->addLayout(historyRow);

    layout->addStretch();

    auto* actions = new QHBoxLayout();
    auto* overlayBtn = new QPushButton(tr("Open overlay"), panel);
    overlayBtn->setObjectName(QStringLiteral("PrimaryButton"));
    connect(overlayBtn, &QPushButton::clicked, this, &SettingsWindow::showOverlayRequested);

    auto* quitBtn = new QPushButton(tr("Quit Spaste"), panel);
    connect(quitBtn, &QPushButton::clicked, this, &SettingsWindow::quitRequested);

    actions->addWidget(overlayBtn);
    actions->addWidget(quitBtn);
    layout->addLayout(actions);

    root->addWidget(panel);
}
