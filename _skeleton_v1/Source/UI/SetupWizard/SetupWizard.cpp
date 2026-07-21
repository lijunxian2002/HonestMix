#include "SetupWizard.h"
#include "StepIndicator.h"
#include "StepHeadphone.h"
#include "StepInterface.h"
#include "StepHabits.h"
#include "AudioInterfaceBrowser.h"
#include "../LookAndFeel.h"

//==============================================================================
SetupWizard::SetupWizard(AppState& state, std::function<void()> complete)
    : appState(state), onComplete(std::move(complete))
{
    // 步骤进度条
    stepIndicator = std::make_unique<StepIndicator>();
    addAndMakeVisible(stepIndicator.get());

    // 步骤 1
    stepHeadphone = std::make_unique<StepHeadphone>();
    stepHeadphone->onSelectionChanged = [this]() { onSelectionChanged(); };
    addAndMakeVisible(stepHeadphone.get());

    // 步骤 2
    stepInterface = std::make_unique<StepInterface>();
    stepInterface->onSelectionChanged = [this]() { onSelectionChanged(); };
    stepInterface->onRequestBrowser = [this]()
    {
        if (interfaceBrowser)
            interfaceBrowser->show();
    };
    addAndMakeVisible(stepInterface.get());

    // 步骤 3
    stepHabits = std::make_unique<StepHabits>();
    stepHabits->onSelectionChanged = [this]() { onSelectionChanged(); };
    stepHabits->onCheckEnvChanged = [this]() { onSelectionChanged(); };
    addAndMakeVisible(stepHabits.get());

    // 声卡浏览器（初始隐藏）
    interfaceBrowser = std::make_unique<AudioInterfaceBrowser>();
    interfaceBrowser->onInterfaceSelected =
        [this](int index, const juce::String& name)
        {
            saveInterfaceSelection(index, name);
            // 选中后自动标记 StepInterface 为已选
            onSelectionChanged();
        };
    addChildComponent(interfaceBrowser.get());
    interfaceBrowser->setVisible(false);

    // 按钮
    backButton.setButtonText(TRANS("上一步"));
    backButton.onClick = [this]()
    {
        if (currentStep > 1)
            goToStep(currentStep - 1);
    };
    addAndMakeVisible(backButton);

    nextButton.setButtonText(TRANS("下一步"));
    nextButton.onClick = [this]() { nextStep(); };
    addAndMakeVisible(nextButton);

    // 显示第一步骤
    showStep(1);
}

SetupWizard::~SetupWizard() = default;

//==============================================================================
void SetupWizard::paint(juce::Graphics& g)
{
    HonestMixLookAndFeel::drawCardBackground(g, getLocalBounds());
}

void SetupWizard::resized()
{
    auto bounds = getLocalBounds().reduced(18);

    // 步骤进度条
    stepIndicator->setBounds(bounds.removeFromTop(14));

    bounds.removeFromTop(12);

    // 步骤内容区域
    auto contentBounds = bounds.removeFromTop(240);
    stepHeadphone->setBounds(contentBounds);
    stepInterface->setBounds(contentBounds);
    stepHabits->setBounds(contentBounds);

    bounds.removeFromTop(8);

    // 按钮行
    auto buttonBounds = bounds.removeFromTop(28);
    backButton.setBounds(buttonBounds.removeFromLeft(80));
    buttonBounds.removeFromLeft(12);
    nextButton.setBounds(buttonBounds.removeFromRight(80));
}

//==============================================================================
void SetupWizard::goToStep(int step)
{
    if (step < 1 || step > 3)
        return;

    currentStep = step;
    stepIndicator->setCurrentStep(step);
    showStep(step);
    updateButtonState();
}

void SetupWizard::nextStep()
{
    if (currentStep < 3)
    {
        // 保存当前步骤的选中数据
        if (currentStep == 1) saveHeadphoneSelection();
        if (currentStep == 2)
        {
            int idx = stepInterface->getSelectedIndex();
            if (idx >= 0)
                saveInterfaceSelection(idx, {});
        }

        goToStep(currentStep + 1);
    }
    else
    {
        // 步骤 3 完成
        saveHabitSelection();
        if (onComplete)
            onComplete();
    }
}

//==============================================================================
void SetupWizard::showStep(int step)
{
    hideAllSteps();

    switch (step)
    {
    case 1: stepHeadphone->setVisible(true); break;
    case 2: stepInterface->setVisible(true); break;
    case 3: stepHabits->setVisible(true);    break;
    }

    backButton.setVisible(step > 1);

    if (step >= 3)
        nextButton.setButtonText(TRANS("完成设置"));
    else
        nextButton.setButtonText(TRANS("下一步"));

    updateButtonState();
}

void SetupWizard::hideAllSteps()
{
    stepHeadphone->setVisible(false);
    stepInterface->setVisible(false);
    stepHabits->setVisible(false);
}

void SetupWizard::updateButtonState()
{
    bool valid = false;

    switch (currentStep)
    {
    case 1: valid = stepHeadphone->isSelectionValid();  break;
    case 2: valid = stepInterface->isSelectionValid();  break;
    case 3: valid = stepHabits->isFullySelected();      break;
    }

    nextButton.setEnabled(valid);
}

void SetupWizard::onSelectionChanged()
{
    updateButtonState();
}

//==============================================================================
// ——— 保存数据到 AppState ———

void SetupWizard::saveHeadphoneSelection()
{
    int idx = stepHeadphone->getSelectedIndex();
    if (idx >= 0)
    {
        appState.setHeadphoneModel(idx);
        appState.setHeadphoneModelName(
            Settings::getHeadphoneName(idx));
    }
}

void SetupWizard::saveInterfaceSelection(int index, const juce::String& name)
{
    appState.setAudioInterface(index);
    if (name.isNotEmpty())
        appState.setAudioInterfaceName(name);
    else if (index >= 0)
        appState.setAudioInterfaceName(
            Settings::getInterfaceName(index));
}

void SetupWizard::saveHabitSelection()
{
    int idx = stepHabits->getSelectedIndex();
    if (idx >= 0)
        appState.setMixingHabit(idx);

    int checkIdx = stepHabits->getCheckEnvIndex();
    if (checkIdx >= 0)
        appState.setCheckOtherEnvironment(checkIdx);
}
