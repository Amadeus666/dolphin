// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>

#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/IPC_HLE/WII_IPC_HLE.h"
#include "DiscIO/Enums.h"
#include "DolphinWX/Config/WiiConfigPane.h"
#include "DolphinWX/DolphinSlider.h"
#include "DolphinWX/WxUtils.h"

WiiConfigPane::WiiConfigPane(wxWindow* parent, wxWindowID id) : wxPanel(parent, id)
{
  InitializeGUI();
  LoadGUIValues();
  RefreshGUI();
}

void WiiConfigPane::InitializeGUI()
{
  m_aspect_ratio_strings.Add("4:3");
  m_aspect_ratio_strings.Add("16:9");

  m_system_language_strings.Add(_("Japanese"));
  m_system_language_strings.Add(_("English"));
  m_system_language_strings.Add(_("German"));
  m_system_language_strings.Add(_("French"));
  m_system_language_strings.Add(_("Spanish"));
  m_system_language_strings.Add(_("Italian"));
  m_system_language_strings.Add(_("Dutch"));
  m_system_language_strings.Add(_("Simplified Chinese"));
  m_system_language_strings.Add(_("Traditional Chinese"));
  m_system_language_strings.Add(_("Korean"));

  m_bt_sensor_bar_pos_strings.Add(_("Bottom"));
  m_bt_sensor_bar_pos_strings.Add(_("Top"));

  m_screensaver_checkbox = new wxCheckBox(this, wxID_ANY, _("Enable Screen Saver"));
  m_pal60_mode_checkbox = new wxCheckBox(this, wxID_ANY, _("Use PAL60 Mode (EuRGB60)"));
  m_aspect_ratio_choice =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_aspect_ratio_strings);
  m_system_language_choice =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_system_language_strings);
  m_sd_card_checkbox = new wxCheckBox(this, wxID_ANY, _("Insert SD Card"));
  m_connect_keyboard_checkbox = new wxCheckBox(this, wxID_ANY, _("Connect USB Keyboard"));
  m_bt_sensor_bar_pos =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_bt_sensor_bar_pos_strings);
  m_bt_sensor_bar_sens = new DolphinSlider(this, wxID_ANY, 0, 0, 4);
  m_bt_speaker_volume = new DolphinSlider(this, wxID_ANY, 0, 0, 127);
  m_bt_wiimote_motor = new wxCheckBox(this, wxID_ANY, _("Wiimote Motor"));

  m_screensaver_checkbox->Bind(wxEVT_CHECKBOX, &WiiConfigPane::OnScreenSaverCheckBoxChanged, this);
  m_pal60_mode_checkbox->Bind(wxEVT_CHECKBOX, &WiiConfigPane::OnPAL60CheckBoxChanged, this);
  m_aspect_ratio_choice->Bind(wxEVT_CHOICE, &WiiConfigPane::OnAspectRatioChoiceChanged, this);
  m_system_language_choice->Bind(wxEVT_CHOICE, &WiiConfigPane::OnSystemLanguageChoiceChanged, this);
  m_sd_card_checkbox->Bind(wxEVT_CHECKBOX, &WiiConfigPane::OnSDCardCheckBoxChanged, this);
  m_connect_keyboard_checkbox->Bind(wxEVT_CHECKBOX,
                                    &WiiConfigPane::OnConnectKeyboardCheckBoxChanged, this);
  m_bt_sensor_bar_pos->Bind(wxEVT_CHOICE, &WiiConfigPane::OnSensorBarPosChanged, this);
  m_bt_sensor_bar_sens->Bind(wxEVT_SLIDER, &WiiConfigPane::OnSensorBarSensChanged, this);
  m_bt_speaker_volume->Bind(wxEVT_SLIDER, &WiiConfigPane::OnSpeakerVolumeChanged, this);
  m_bt_wiimote_motor->Bind(wxEVT_CHECKBOX, &WiiConfigPane::OnWiimoteMotorChanged, this);

  m_screensaver_checkbox->SetToolTip(_("Dims the screen after five minutes of inactivity."));
  m_pal60_mode_checkbox->SetToolTip(_("Sets the Wii display mode to 60Hz (480i) instead of 50Hz "
                                      "(576i) for PAL games.\nMay not work for all games."));
  m_system_language_choice->SetToolTip(_("Sets the Wii system language."));
  m_sd_card_checkbox->SetToolTip(_("Saved to /Wii/sd.raw (default size is 128mb)"));
  m_connect_keyboard_checkbox->SetToolTip(_("May cause slow down in Wii Menu and some games."));

  const int space5 = FromDIP(5);

  wxGridBagSizer* const misc_settings_grid_sizer = new wxGridBagSizer(space5, space5);
  misc_settings_grid_sizer->Add(m_screensaver_checkbox, wxGBPosition(0, 0), wxGBSpan(1, 2));
  misc_settings_grid_sizer->Add(m_pal60_mode_checkbox, wxGBPosition(1, 0), wxGBSpan(1, 2));
  misc_settings_grid_sizer->Add(new wxStaticText(this, wxID_ANY, _("Aspect Ratio:")),
                                wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
  misc_settings_grid_sizer->Add(m_aspect_ratio_choice, wxGBPosition(2, 1), wxDefaultSpan,
                                wxALIGN_CENTER_VERTICAL);
  misc_settings_grid_sizer->Add(new wxStaticText(this, wxID_ANY, _("System Language:")),
                                wxGBPosition(3, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
  misc_settings_grid_sizer->Add(m_system_language_choice, wxGBPosition(3, 1), wxDefaultSpan,
                                wxALIGN_CENTER_VERTICAL);

  auto* const bt_sensor_bar_pos_sizer = new wxBoxSizer(wxHORIZONTAL);
  bt_sensor_bar_pos_sizer->Add(new wxStaticText(this, wxID_ANY, _("Min")), 0,
                               wxALIGN_CENTER_VERTICAL);
  bt_sensor_bar_pos_sizer->Add(m_bt_sensor_bar_sens, 0, wxALIGN_CENTER_VERTICAL);
  bt_sensor_bar_pos_sizer->Add(new wxStaticText(this, wxID_ANY, _("Max")), 0,
                               wxALIGN_CENTER_VERTICAL);

  auto* const bt_speaker_volume_sizer = new wxBoxSizer(wxHORIZONTAL);
  bt_speaker_volume_sizer->Add(new wxStaticText(this, wxID_ANY, _("Min")), 0,
                               wxALIGN_CENTER_VERTICAL);
  bt_speaker_volume_sizer->Add(m_bt_speaker_volume, 0, wxALIGN_CENTER_VERTICAL);
  bt_speaker_volume_sizer->Add(new wxStaticText(this, wxID_ANY, _("Max")), 0,
                               wxALIGN_CENTER_VERTICAL);

  wxGridBagSizer* const bt_settings_grid_sizer = new wxGridBagSizer(space5, space5);
  bt_settings_grid_sizer->Add(new wxStaticText(this, wxID_ANY, _("Sensor Bar Position:")),
                              wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
  bt_settings_grid_sizer->Add(m_bt_sensor_bar_pos, wxGBPosition(0, 1), wxDefaultSpan,
                              wxALIGN_CENTER_VERTICAL);
  bt_settings_grid_sizer->Add(new wxStaticText(this, wxID_ANY, _("IR Sensitivity:")),
                              wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
  bt_settings_grid_sizer->Add(bt_sensor_bar_pos_sizer, wxGBPosition(1, 1), wxDefaultSpan,
                              wxALIGN_CENTER_VERTICAL);
  bt_settings_grid_sizer->Add(new wxStaticText(this, wxID_ANY, _("Speaker Volume:")),
                              wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
  bt_settings_grid_sizer->Add(bt_speaker_volume_sizer, wxGBPosition(2, 1), wxDefaultSpan,
                              wxALIGN_CENTER_VERTICAL);
  bt_settings_grid_sizer->Add(m_bt_wiimote_motor, wxGBPosition(3, 0), wxGBSpan(1, 2),
                              wxALIGN_CENTER_VERTICAL);

  wxStaticBoxSizer* const misc_settings_static_sizer =
      new wxStaticBoxSizer(wxVERTICAL, this, _("Misc Settings"));
  misc_settings_static_sizer->AddSpacer(space5);
  misc_settings_static_sizer->Add(misc_settings_grid_sizer, 0, wxLEFT | wxRIGHT, space5);
  misc_settings_static_sizer->AddSpacer(space5);

  wxStaticBoxSizer* const device_settings_sizer =
      new wxStaticBoxSizer(wxVERTICAL, this, _("Device Settings"));
  device_settings_sizer->AddSpacer(space5);
  device_settings_sizer->Add(m_sd_card_checkbox, 0, wxLEFT | wxRIGHT, space5);
  device_settings_sizer->AddSpacer(space5);
  device_settings_sizer->Add(m_connect_keyboard_checkbox, 0, wxLEFT | wxRIGHT, space5);
  device_settings_sizer->AddSpacer(space5);

  auto* const bt_settings_static_sizer =
      new wxStaticBoxSizer(wxVERTICAL, this, _("Wii Remote Settings"));
  bt_settings_static_sizer->AddSpacer(space5);
  bt_settings_static_sizer->Add(bt_settings_grid_sizer, 0, wxLEFT | wxRIGHT, space5);
  bt_settings_static_sizer->AddSpacer(space5);

  wxBoxSizer* const main_sizer = new wxBoxSizer(wxVERTICAL);
  main_sizer->AddSpacer(space5);
  main_sizer->Add(misc_settings_static_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT, space5);
  main_sizer->AddSpacer(space5);
  main_sizer->Add(device_settings_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT, space5);
  main_sizer->AddSpacer(space5);
  main_sizer->Add(bt_settings_static_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT, space5);
  main_sizer->AddSpacer(space5);

  SetSizer(main_sizer);
}

void WiiConfigPane::LoadGUIValues()
{
  m_screensaver_checkbox->SetValue(!!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.SSV"));
  m_pal60_mode_checkbox->SetValue(SConfig::GetInstance().bPAL60);
  m_aspect_ratio_choice->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.AR"));
  m_system_language_choice->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.LNG"));

  m_sd_card_checkbox->SetValue(SConfig::GetInstance().m_WiiSDCard);
  m_connect_keyboard_checkbox->SetValue(SConfig::GetInstance().m_WiiKeyboard);

  m_bt_sensor_bar_pos->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("BT.BAR"));
  m_bt_sensor_bar_sens->SetValue(SConfig::GetInstance().m_SYSCONF->GetData<u32>("BT.SENS"));
  m_bt_speaker_volume->SetValue(SConfig::GetInstance().m_SYSCONF->GetData<u8>("BT.SPKV"));
  m_bt_wiimote_motor->SetValue(SConfig::GetInstance().m_SYSCONF->GetData<bool>("BT.MOT"));
}

void WiiConfigPane::RefreshGUI()
{
  if (Core::IsRunning())
  {
    m_screensaver_checkbox->Disable();
    m_pal60_mode_checkbox->Disable();
    m_aspect_ratio_choice->Disable();
    m_system_language_choice->Disable();

    m_bt_sensor_bar_pos->Disable();
    m_bt_sensor_bar_sens->Disable();
    m_bt_speaker_volume->Disable();
    m_bt_wiimote_motor->Disable();
  }
}

void WiiConfigPane::OnScreenSaverCheckBoxChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_SYSCONF->SetData("IPL.SSV", m_screensaver_checkbox->IsChecked());
}

void WiiConfigPane::OnPAL60CheckBoxChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().bPAL60 = m_pal60_mode_checkbox->IsChecked();
  SConfig::GetInstance().m_SYSCONF->SetData("IPL.E60", m_pal60_mode_checkbox->IsChecked());
}

void WiiConfigPane::OnSDCardCheckBoxChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_WiiSDCard = m_sd_card_checkbox->IsChecked();
  WII_IPC_HLE_Interface::SDIO_EventNotify();
}

void WiiConfigPane::OnConnectKeyboardCheckBoxChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_WiiKeyboard = m_connect_keyboard_checkbox->IsChecked();
}

void WiiConfigPane::OnSystemLanguageChoiceChanged(wxCommandEvent& event)
{
  DiscIO::Language wii_system_lang =
      static_cast<DiscIO::Language>(m_system_language_choice->GetSelection());
  SConfig::GetInstance().m_SYSCONF->SetData("IPL.LNG", wii_system_lang);
  u8 country_code = GetSADRCountryCode(wii_system_lang);

  if (!SConfig::GetInstance().m_SYSCONF->SetArrayData("IPL.SADR", &country_code, 1))
    WxUtils::ShowErrorDialog(_("Failed to update country code in SYSCONF"));
}

void WiiConfigPane::OnAspectRatioChoiceChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_SYSCONF->SetData("IPL.AR", m_aspect_ratio_choice->GetSelection());
}

void WiiConfigPane::OnSensorBarPosChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_SYSCONF->SetData("BT.BAR", event.GetInt());
}

void WiiConfigPane::OnSensorBarSensChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_SYSCONF->SetData("BT.SENS", event.GetInt());
}

void WiiConfigPane::OnSpeakerVolumeChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_SYSCONF->SetData("BT.SPKV", event.GetInt());
}

void WiiConfigPane::OnWiimoteMotorChanged(wxCommandEvent& event)
{
  SConfig::GetInstance().m_SYSCONF->SetData("BT.MOT", event.GetInt());
}

// Change from IPL.LNG value to IPL.SADR country code.
// http://wiibrew.org/wiki/Country_Codes
u8 WiiConfigPane::GetSADRCountryCode(DiscIO::Language language)
{
  switch (language)
  {
  case DiscIO::Language::LANGUAGE_JAPANESE:
    return 1;  // Japan
  case DiscIO::Language::LANGUAGE_ENGLISH:
    return 49;  // USA
  case DiscIO::Language::LANGUAGE_GERMAN:
    return 78;  // Germany
  case DiscIO::Language::LANGUAGE_FRENCH:
    return 77;  // France
  case DiscIO::Language::LANGUAGE_SPANISH:
    return 105;  // Spain
  case DiscIO::Language::LANGUAGE_ITALIAN:
    return 83;  // Italy
  case DiscIO::Language::LANGUAGE_DUTCH:
    return 94;  // Netherlands
  case DiscIO::Language::LANGUAGE_SIMPLIFIED_CHINESE:
  case DiscIO::Language::LANGUAGE_TRADITIONAL_CHINESE:
    return 157;  // China
  case DiscIO::Language::LANGUAGE_KOREAN:
    return 136;  // Korea
  case DiscIO::Language::LANGUAGE_UNKNOWN:
    break;
  }

  PanicAlert("Invalid language. Defaulting to Japanese.");
  return 1;
}
