/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ui_appearance_ability_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ui_appearance_ability_proxy.h"
#include "ui_appearance_log.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
sptr<UiAppearanceAbilityClient> UiAppearanceAbilityClient::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (!instance_) {
            instance_ = new UiAppearanceAbilityClient;
            uiAppearanceServiceProxy_ = CreateUiAppearanceServiceProxy();
        }
    }
    return instance_;
}

sptr<UiAppearanceAbilityInterface> UiAppearanceAbilityClient::GetUiAppearanceServiceProxy()
{
    if (uiAppearanceServiceProxy_ == nullptr) {
        HILOG_ERROR("Redo CreateUiAppearanceServiceProxy");
        uiAppearanceServiceProxy_ = CreateUiAppearanceServiceProxy();
    }
    return uiAppearanceServiceProxy_;
}

int32_t UiAppearanceAbilityClient::SetDarkMode(UiAppearanceAbilityInterface::DarkMode mode)
{
    if (!GetUiAppearanceServiceProxy()) {
        HILOG_ERROR("SetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::ErrCode::SYS_ERR;
    }
    return uiAppearanceServiceProxy_->SetDarkMode(mode);
}

UiAppearanceAbilityInterface::DarkMode UiAppearanceAbilityClient::GetDarkMode()
{
    if (!GetUiAppearanceServiceProxy()) {
        HILOG_ERROR("GetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::DarkMode::UNKNOWN;
    }
    return uiAppearanceServiceProxy_->GetDarkMode();
}

sptr<UiAppearanceAbilityInterface> UiAppearanceAbilityClient::CreateUiAppearanceServiceProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        HILOG_ERROR("Get SystemAbilityManager failed.");
        return nullptr;
    }

    sptr<IRemoteObject> systemAbility = systemAbilityManager->GetSystemAbility(ARKUI_UI_APPEARANCE_SERVICE_ID);
    if (systemAbility == nullptr) {
        HILOG_ERROR("Get SystemAbility failed.");
        return nullptr;
    }

    deathRecipient_ = new UiAppearanceDeathRecipient;
    systemAbility->AddDeathRecipient(deathRecipient_);
    sptr<UiAppearanceAbilityInterface> uiAppearanceServiceProxy =
        iface_cast<UiAppearanceAbilityInterface>(systemAbility);
    if (uiAppearanceServiceProxy == nullptr) {
        HILOG_ERROR("Get uiAppearanceServiceProxy from SA failed.");
        return nullptr;
    }
    HILOG_INFO("Get uiAppearanceServiceProxy successful.");
    return uiAppearanceServiceProxy;
}

void UiAppearanceAbilityClient::OnRemoteSaDied(const wptr<IRemoteObject>& remote)
{
    // Used for new connections after the service may be disconnected.
    uiAppearanceServiceProxy_ = CreateUiAppearanceServiceProxy();
}

void UiAppearanceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& object)
{
    HILOG_INFO("UiAppearanceDeathRecipient on remote systemAbility died.");
    UiAppearanceAbilityClient::GetInstance()->OnRemoteSaDied(object);
}
} // namespace ArkUi::UiAppearance
} // namespace OHOS
