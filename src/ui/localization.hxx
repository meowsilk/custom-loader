#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace loader::ui
{

enum class Language : int
{
    English = 0,
    German = 1,
    Spanish = 2,
    Russian = 3,
    French = 4,
    Portuguese = 5,
    Turkish = 6,
    Polish = 7,
    Italian = 8,
};

inline constexpr std::size_t kLanguageCount = 9;

inline constexpr std::array<Language, kLanguageCount> kLanguages = {Language::English,
                                                                    Language::German,
                                                                    Language::Spanish,
                                                                    Language::Russian,
                                                                    Language::French,
                                                                    Language::Portuguese,
                                                                    Language::Turkish,
                                                                    Language::Polish,
                                                                    Language::Italian};

enum class Str : int
{
    LoginTitle,
    LoginSubtitle,
    Username,
    Password,
    ConfirmPassword,
    RememberMe,
    CreateAccount,
    SignIn,
    AccountCreated,
    RegisterTitle,
    RegisterSubtitle,
    BackToSignIn,
    WelcomeGreeting,
    WelcomeTagline,
    StagePreparing,
    StageSyncing,
    StageAlmostReady,
    OurProducts,
    UsernameEmpty,
    UsernameTooShort,
    UsernameTooLong,
    UsernameInvalid,
    UsernameCheck,
    PasswordEmpty,
    PasswordTooShort,
    PasswordTooLong,
    PasswordCheck,
    ConfirmEmpty,
    ConfirmMismatch,
    ConfirmCheck,
    LicenseEmpty,
    LicenseLength,
    LicenseChars,
    LicenseCheck,
    AuthInvalid,
    AuthLocked,
    AuthNetwork,
    AuthLicenseInvalid,
    AuthUsernameTaken,
    AuthGeneric,
    DescCounterStrike2,
    DescEscapeFromTarkov,
    DescFortnite,
    DescApexLegends,
    DescDayZ,
    InfoInjectionType,
    InfoLastUpdate,
    InfoLastDetection,
    InjectionInternal,
    InjectionExternal,
    DetectionNever,
    KeyPrompt,
    Activate,
    KeyAccepted,
    Load,
    InfoLicense,
    DurationDay,
    DurationWeek,
    DurationLifetime,
    DescRust,
    DescPubg,
    DescRainbowSix,
    DescCallOfDuty,
    DescGtaV,
    Count,
};

inline constexpr std::size_t kStrCount = static_cast<std::size_t>(Str::Count);

std::u32string_view languageName(Language language);
std::string_view languageCode(Language language);
std::optional<Language> languageFromCode(std::string_view code);
std::u32string_view translate(Language language, Str id);

class Localization
{
public:
    class Subscription
    {
    public:
        Subscription() = default;
        Subscription(Localization& owner, std::size_t id) : owner_(&owner), id_(id) {}
        ~Subscription() { release(); }

        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;
        Subscription(Subscription&& other) noexcept
            : owner_(std::exchange(other.owner_, nullptr)), id_(other.id_)
        {
        }
        Subscription& operator=(Subscription&& other) noexcept
        {
            if (this != &other)
            {
                release();
                owner_ = std::exchange(other.owner_, nullptr);
                id_ = other.id_;
            }
            return *this;
        }

    private:
        void release()
        {
            if (owner_ != nullptr)
                owner_->unsubscribe(id_);
            owner_ = nullptr;
        }

        Localization* owner_ = nullptr;
        std::size_t id_ = 0;
    };

    Language language() const { return language_; }
    void setLanguage(Language language);
    std::u32string_view text(Str id) const { return translate(language_, id); }

    [[nodiscard]] Subscription subscribe(std::function<void()> listener);

private:
    void unsubscribe(std::size_t id);

    Language language_ = Language::English;
    std::vector<std::pair<std::size_t, std::function<void()>>> listeners_;
    std::size_t nextId_ = 1;
};

class TextBinding
{
public:
    TextBinding() = default;
    TextBinding(Localization& localization, Str id, std::function<void(std::u32string_view)> apply)
        : subscription_(localization.subscribe([&localization, id, apply = std::move(apply)]
                                               { apply(localization.text(id)); }))
    {
    }

private:
    Localization::Subscription subscription_;
};

}
