#include "ui/localization.hxx"

#include <algorithm>

namespace loader::ui
{
namespace
{

using Table = std::array<const char32_t*, kStrCount>;

constexpr Table kEnglish = {
    U"Welcome back",
    U"Sign in to continue to loader.gg",
    U"Username",
    U"Password",
    U"Confirm password",
    U"Remember me",
    U"Create account",
    U"Sign in",
    U"Account created. Sign in to continue.",
    U"Create your account",
    U"Choose a username and password",
    U"Back to sign in",
    U"Welcome back",
    U"Are you ready for the experience?",
    U"Preparing your workspace",
    U"Syncing your products",
    U"Almost ready",
    U"Our products",
    U"Enter your username.",
    U"Username must be at least 3 characters.",
    U"Username can be at most 32 characters.",
    U"Use only letters, numbers, dots, dashes and underscores.",
    U"Check your username.",
    U"Enter your password.",
    U"Password must be at least 6 characters.",
    U"Password can be at most 128 characters.",
    U"Check your password.",
    U"Confirm your password.",
    U"Passwords do not match.",
    U"Check the password confirmation.",
    U"Enter your license key.",
    U"License key must have 16 characters.",
    U"License key can only contain letters and numbers.",
    U"Check your license key.",
    U"Incorrect username or password.",
    U"This account is locked. Try again later.",
    U"Can't reach the server. Check your connection.",
    U"This license key is not valid.",
    U"That username is already taken.",
    U"Something went wrong.",
    U"Counter-Strike 2 is Valve's tactical team shooter. Plant or defuse the bomb, master the economy and "
    U"out-aim your opponents in fast competitive rounds.",
    U"Escape from Tarkov is a hardcore extraction shooter. Raid contested maps, loot gear and get out alive "
    U"before the raid timer runs out.",
    U"Fortnite is Epic Games' free-to-play battle royale. Drop in with up to 99 other players, build to "
    U"survive and be the last one standing.",
    U"Apex Legends is a fast-paced hero shooter from Respawn. Pick a Legend with unique abilities, squad up "
    U"and fight to become the champion.",
    U"DayZ is an open-world survival sandbox by Bohemia Interactive. Scavenge for supplies and stay alive "
    U"among the infected and other survivors.",
    U"Injection type",
    U"Last update",
    U"Last detection",
    U"Internal",
    U"External",
    U"never",
    U"Enter your license key",
    U"Activate",
    U"Key accepted.",
    U"Load",
    U"License",
    U"1 day",
    U"1 week",
    U"Lifetime",
    U"Rust is a hardcore multiplayer survival game. Gather resources, build a base and fight other players "
    U"to survive on a hostile island.",
    U"PUBG: Battlegrounds is a large-scale battle royale. Parachute in with 99 other players, scavenge "
    U"weapons and outlast everyone.",
    U"Rainbow Six Siege is a tactical team shooter built around destruction and planning. Breach, defend and "
    U"outsmart the other team as an elite operator.",
    U"Call of Duty delivers explosive military action with fast-paced multiplayer and large-scale Warzone "
    U"battles. Gear up and drop in.",
    U"Grand Theft Auto V is an open-world action adventure set in Los Santos. Play three criminals pulling "
    U"off heists, or take the chaos online.",
};

constexpr Table kGerman = {
    U"Willkommen zurück",
    U"Melde dich an, um bei loader.gg fortzufahren",
    U"Benutzername",
    U"Passwort",
    U"Passwort bestätigen",
    U"Angemeldet bleiben",
    U"Konto erstellen",
    U"Anmelden",
    U"Konto erstellt. Melde dich an, um fortzufahren.",
    U"Erstelle dein Konto",
    U"Wähle einen Benutzernamen und ein Passwort",
    U"Zurück zur Anmeldung",
    U"Willkommen zurück",
    U"Bist du bereit für das Erlebnis?",
    U"Arbeitsbereich wird vorbereitet",
    U"Produkte werden synchronisiert",
    U"Fast fertig",
    U"Unsere Produkte",
    U"Gib deinen Benutzernamen ein.",
    U"Benutzername: mindestens 3 Zeichen.",
    U"Benutzername: höchstens 32 Zeichen.",
    U"Nur Buchstaben, Zahlen, Punkt, - und _ erlaubt.",
    U"Überprüfe deinen Benutzernamen.",
    U"Gib dein Passwort ein.",
    U"Passwort: mindestens 6 Zeichen.",
    U"Passwort: höchstens 128 Zeichen.",
    U"Überprüfe dein Passwort.",
    U"Bestätige dein Passwort.",
    U"Die Passwörter stimmen nicht überein.",
    U"Überprüfe die Passwortbestätigung.",
    U"Gib deinen Lizenzschlüssel ein.",
    U"Lizenzschlüssel: genau 16 Zeichen.",
    U"Lizenzschlüssel: nur Buchstaben und Zahlen.",
    U"Überprüfe deinen Lizenzschlüssel.",
    U"Benutzername oder Passwort falsch.",
    U"Konto gesperrt. Versuche es später erneut.",
    U"Server nicht erreichbar. Prüfe deine Verbindung.",
    U"Dieser Lizenzschlüssel ist ungültig.",
    U"Dieser Benutzername ist bereits vergeben.",
    U"Etwas ist schiefgelaufen.",
    U"Counter-Strike 2 ist Valves taktischer Team-Shooter. Platziere oder entschärfe die Bombe, beherrsche "
    U"die Wirtschaft und besiege deine Gegner in schnellen Wettkampfrunden.",
    U"Escape from Tarkov ist ein knallharter Extraction-Shooter. Durchstreife umkämpfte Karten, sammle "
    U"Ausrüstung und entkomme lebend, bevor der Raid-Timer abläuft.",
    U"Fortnite ist das kostenlose Battle Royale von Epic Games. Spring mit bis zu 99 anderen Spielern ab, "
    U"baue, um zu überleben, und sei der Letzte, der steht.",
    U"Apex Legends ist ein rasanter Hero-Shooter von Respawn. Wähle eine Legende mit einzigartigen "
    U"Fähigkeiten, bilde ein Team und kämpfe um den Titel des Champions.",
    U"DayZ ist eine Open-World-Survival-Sandbox von Bohemia Interactive. Suche nach Vorräten und überlebe "
    U"zwischen Infizierten und anderen Überlebenden.",
    U"Injektionstyp",
    U"Letztes Update",
    U"Letzte Erkennung",
    U"Intern",
    U"Extern",
    U"nie",
    U"Dein Lizenzschlüssel",
    U"Aktivieren",
    U"Schlüssel akzeptiert.",
    U"Laden",
    U"Lizenz",
    U"1 Tag",
    U"1 Woche",
    U"Lebenslang",
    U"Rust ist ein knallhartes Multiplayer-Survivalspiel. Sammle Ressourcen, baue eine Basis und kämpfe "
    U"gegen andere Spieler ums Überleben auf einer feindlichen Insel.",
    U"PUBG: Battlegrounds ist ein großangelegtes Battle Royale. Springe mit 99 anderen Spielern ab, sammle "
    U"Waffen und überlebe alle.",
    U"Rainbow Six Siege ist ein taktischer Team-Shooter rund um Zerstörung und Planung. Brich ein, "
    U"verteidige und überlist das gegnerische Team als Elite-Operator.",
    U"Call of Duty bietet explosive Militäraction mit rasantem Multiplayer und riesigen Warzone-Schlachten. "
    U"Rüste dich aus und spring rein.",
    U"Grand Theft Auto V ist ein Open-World-Actionabenteuer in Los Santos. Spiele drei Kriminelle bei großen "
    U"Raubzügen oder stürze dich online ins Chaos.",
};

constexpr Table kSpanish = {
    U"Bienvenido de nuevo",
    U"Inicia sesión para continuar en loader.gg",
    U"Usuario",
    U"Contraseña",
    U"Confirmar contraseña",
    U"Recordarme",
    U"Crear cuenta",
    U"Iniciar sesión",
    U"Cuenta creada. Inicia sesión para continuar.",
    U"Crea tu cuenta",
    U"Elige un usuario y una contraseña",
    U"Volver a iniciar sesión",
    U"Bienvenido de nuevo",
    U"¿Listo para la experiencia?",
    U"Preparando tu espacio de trabajo",
    U"Sincronizando tus productos",
    U"Casi listo",
    U"Nuestros productos",
    U"Introduce tu usuario.",
    U"El usuario necesita al menos 3 caracteres.",
    U"El usuario admite como máximo 32 caracteres.",
    U"Solo letras, números, puntos, guiones y _.",
    U"Revisa tu usuario.",
    U"Introduce tu contraseña.",
    U"La contraseña necesita al menos 6 caracteres.",
    U"La contraseña admite como máximo 128 caracteres.",
    U"Revisa tu contraseña.",
    U"Confirma tu contraseña.",
    U"Las contraseñas no coinciden.",
    U"Revisa la confirmación de la contraseña.",
    U"Introduce tu clave de licencia.",
    U"La clave debe tener 16 caracteres.",
    U"La clave solo admite letras y números.",
    U"Revisa tu clave de licencia.",
    U"Usuario o contraseña incorrectos.",
    U"Cuenta bloqueada. Inténtalo más tarde.",
    U"Sin conexión con el servidor. Revisa tu red.",
    U"Esta clave de licencia no es válida.",
    U"Ese nombre de usuario ya está en uso.",
    U"Algo salió mal.",
    U"Counter-Strike 2 es el shooter táctico por equipos de Valve. Planta o desactiva la bomba, domina la "
    U"economía y supera a tus rivales en rondas competitivas muy rápidas.",
    U"Escape from Tarkov es un shooter de extracción muy exigente. Explora mapas disputados, consigue equipo "
    U"y sal con vida antes de que acabe el tiempo de la incursión.",
    U"Fortnite es el battle royale gratuito de Epic Games. Salta con hasta 99 jugadores más, construye para "
    U"sobrevivir y sé el último en pie.",
    U"Apex Legends es un hero shooter trepidante de Respawn. Elige una Leyenda con habilidades únicas, forma "
    U"un escuadrón y lucha por ser el campeón.",
    U"DayZ es un sandbox de supervivencia de mundo abierto de Bohemia Interactive. Busca suministros y "
    U"sobrevive entre infectados y otros supervivientes.",
    U"Tipo de inyección",
    U"Última actualización",
    U"Última detección",
    U"Interno",
    U"Externo",
    U"nunca",
    U"Tu clave de licencia",
    U"Activar",
    U"Clave aceptada.",
    U"Cargar",
    U"Licencia",
    U"1 día",
    U"1 semana",
    U"De por vida",
    U"Rust es un implacable juego de supervivencia multijugador. Reúne recursos, construye una base y lucha "
    U"contra otros jugadores en una isla hostil.",
    U"PUBG: Battlegrounds es un battle royale a gran escala. Salta en paracaídas con otros 99 jugadores, "
    U"consigue armas y sobrevive a todos.",
    U"Rainbow Six Siege es un shooter táctico por equipos basado en la destrucción y la planificación. "
    U"Irrumpe, defiende y burla al equipo rival como operador de élite.",
    U"Call of Duty ofrece acción militar explosiva, multijugador trepidante y grandes batallas en Warzone. "
    U"Equípate y lánzate a la partida.",
    U"Grand Theft Auto V es una aventura de acción de mundo abierto en Los Santos. Juega con tres criminales "
    U"en grandes golpes o sumérgete en el caos online.",
};

constexpr Table kRussian = {
    U"С возвращением",
    U"Войдите, чтобы продолжить в loader.gg",
    U"Имя пользователя",
    U"Пароль",
    U"Подтвердите пароль",
    U"Запомнить меня",
    U"Создать аккаунт",
    U"Войти",
    U"Аккаунт создан. Войдите, чтобы продолжить.",
    U"Создайте аккаунт",
    U"Выберите имя пользователя и пароль",
    U"Вернуться ко входу",
    U"С возвращением",
    U"Готовы к новому опыту?",
    U"Подготовка рабочего пространства",
    U"Синхронизация продуктов",
    U"Почти готово",
    U"Наши продукты",
    U"Введите имя пользователя.",
    U"Имя пользователя: минимум 3 символа.",
    U"Имя пользователя: максимум 32 символа.",
    U"Только буквы, цифры, точки, дефисы и _.",
    U"Проверьте имя пользователя.",
    U"Введите пароль.",
    U"Пароль: минимум 6 символов.",
    U"Пароль: максимум 128 символов.",
    U"Проверьте пароль.",
    U"Подтвердите пароль.",
    U"Пароли не совпадают.",
    U"Проверьте подтверждение пароля.",
    U"Введите лицензионный ключ.",
    U"Ключ должен содержать 16 символов.",
    U"Ключ: только буквы и цифры.",
    U"Проверьте лицензионный ключ.",
    U"Неверное имя пользователя или пароль.",
    U"Аккаунт заблокирован. Повторите позже.",
    U"Нет связи с сервером. Проверьте сеть.",
    U"Этот лицензионный ключ недействителен.",
    U"Это имя пользователя уже занято.",
    U"Что-то пошло не так.",
    U"Counter-Strike 2 — тактический командный шутер от Valve. Устанавливайте или обезвреживайте бомбу, "
    U"управляйте экономикой и побеждайте соперников в быстрых соревновательных раундах.",
    U"Escape from Tarkov — хардкорный шутер с эвакуацией. Исследуйте спорные карты, собирайте снаряжение и "
    U"выбирайтесь живым до окончания таймера рейда.",
    U"Fortnite — бесплатная королевская битва от Epic Games. Высаживайтесь с ещё 99 игроками, стройте, чтобы "
    U"выжить, и останьтесь последним.",
    U"Apex Legends — динамичный геройский шутер от Respawn. Выберите легенду с уникальными способностями, "
    U"соберите отряд и сражайтесь за звание чемпиона.",
    U"DayZ — выживание в открытом мире от Bohemia Interactive. Ищите припасы и оставайтесь в живых среди "
    U"заражённых и других выживших.",
    U"Тип инжекта",
    U"Последнее обновление",
    U"Последнее обнаружение",
    U"Внутренний",
    U"Внешний",
    U"никогда",
    U"Лицензионный ключ",
    U"Активировать",
    U"Ключ принят.",
    U"Загрузить",
    U"Лицензия",
    U"1 день",
    U"1 неделя",
    U"Навсегда",
    U"Rust — жестокая многопользовательская игра на выживание. Добывайте ресурсы, стройте базу и сражайтесь "
    U"с другими игроками на враждебном острове.",
    U"PUBG: Battlegrounds — масштабная королевская битва. Высаживайтесь с парашютом вместе с 99 игроками, "
    U"ищите оружие и переживите всех.",
    U"Rainbow Six Siege — тактический командный шутер о разрушении и планировании. Штурмуйте, обороняйтесь и "
    U"перехитрите команду соперников в роли элитного оперативника.",
    U"Call of Duty — взрывной военный экшен со стремительным мультиплеером и масштабными сражениями в "
    U"Warzone. Экипируйтесь и прыгайте в бой.",
    U"Grand Theft Auto V — приключенческий экшен с открытым миром в Лос-Сантосе. Играйте за троих "
    U"преступников, проворачивающих ограбления, или окунитесь в хаос онлайн.",
};

constexpr Table kFrench = {
    U"Content de vous revoir",
    U"Connectez-vous pour continuer sur loader.gg",
    U"Nom d'utilisateur",
    U"Mot de passe",
    U"Confirmer le mot de passe",
    U"Se souvenir de moi",
    U"Créer un compte",
    U"Se connecter",
    U"Compte créé. Connectez-vous pour continuer.",
    U"Créez votre compte",
    U"Choisissez un nom d'utilisateur et un mot de passe",
    U"Retour à la connexion",
    U"Content de vous revoir",
    U"Prêt pour l'expérience ?",
    U"Préparation de votre espace",
    U"Synchronisation des produits",
    U"Presque prêt",
    U"Nos produits",
    U"Saisissez votre nom d'utilisateur.",
    U"Nom d'utilisateur : 3 caractères minimum.",
    U"Nom d'utilisateur : 32 caractères maximum.",
    U"Lettres, chiffres, points, tirets et _ uniquement.",
    U"Vérifiez votre nom d'utilisateur.",
    U"Saisissez votre mot de passe.",
    U"Mot de passe : 6 caractères minimum.",
    U"Mot de passe : 128 caractères maximum.",
    U"Vérifiez votre mot de passe.",
    U"Confirmez votre mot de passe.",
    U"Les mots de passe ne correspondent pas.",
    U"Vérifiez la confirmation du mot de passe.",
    U"Saisissez votre clé de licence.",
    U"La clé doit contenir 16 caractères.",
    U"La clé n'accepte que lettres et chiffres.",
    U"Vérifiez votre clé de licence.",
    U"Nom d'utilisateur ou mot de passe incorrect.",
    U"Compte verrouillé. Réessayez plus tard.",
    U"Serveur injoignable. Vérifiez votre connexion.",
    U"Cette clé de licence n'est pas valide.",
    U"Ce nom d'utilisateur est déjà pris.",
    U"Une erreur s'est produite.",
    U"Counter-Strike 2 est le jeu de tir tactique en équipe de Valve. Posez ou désamorcez la bombe, "
    U"maîtrisez l'économie et surpassez vos adversaires lors de manches compétitives rapides.",
    U"Escape from Tarkov est un jeu de tir d'extraction hardcore. Explorez des cartes disputées, récupérez "
    U"du matériel et repartez vivant avant la fin du raid.",
    U"Fortnite est le battle royale gratuit d'Epic Games. Atterrissez avec 99 autres joueurs, construisez "
    U"pour survivre et soyez le dernier debout.",
    U"Apex Legends est un jeu de tir de héros rapide signé Respawn. Choisissez une Légende aux capacités "
    U"uniques, formez une escouade et devenez champion.",
    U"DayZ est un bac à sable de survie en monde ouvert de Bohemia Interactive. Cherchez des provisions et "
    U"survivez parmi les infectés et les autres survivants.",
    U"Type d'injection",
    U"Dernière mise à jour",
    U"Dernière détection",
    U"Interne",
    U"Externe",
    U"jamais",
    U"Votre clé de licence",
    U"Activer",
    U"Clé acceptée.",
    U"Charger",
    U"Licence",
    U"1 jour",
    U"1 semaine",
    U"À vie",
    U"Rust est un jeu de survie multijoueur impitoyable. Récoltez des ressources, construisez une base et "
    U"affrontez d'autres joueurs sur une île hostile.",
    U"PUBG: Battlegrounds est un battle royale à grande échelle. Sautez en parachute avec 99 autres joueurs, "
    U"trouvez des armes et survivez jusqu’au bout.",
    U"Rainbow Six Siege est un jeu de tir tactique axé sur la destruction et la planification. Attaquez, "
    U"défendez et déjouez l'équipe adverse avec un opérateur d'élite.",
    U"Call of Duty offre une action militaire explosive, un multijoueur rapide et de grandes batailles "
    U"Warzone. Équipez-vous et lancez-vous.",
    U"Grand Theft Auto V est une aventure d'action en monde ouvert à Los Santos. Incarnez trois criminels "
    U"qui enchaînent les braquages, ou plongez dans le chaos en ligne.",
};

constexpr Table kPortuguese = {
    U"Bem-vindo de volta",
    U"Entre para continuar no loader.gg",
    U"Usuário",
    U"Senha",
    U"Confirmar senha",
    U"Lembrar de mim",
    U"Criar conta",
    U"Entrar",
    U"Conta criada. Entre para continuar.",
    U"Crie sua conta",
    U"Escolha um usuário e uma senha",
    U"Voltar para o login",
    U"Bem-vindo de volta",
    U"Pronto para a experiência?",
    U"Preparando seu espaço de trabalho",
    U"Sincronizando seus produtos",
    U"Quase pronto",
    U"Nossos produtos",
    U"Digite seu usuário.",
    U"Usuário: mínimo de 3 caracteres.",
    U"Usuário: máximo de 32 caracteres.",
    U"Use apenas letras, números, pontos, hifens e _.",
    U"Verifique seu usuário.",
    U"Digite sua senha.",
    U"Senha: mínimo de 6 caracteres.",
    U"Senha: máximo de 128 caracteres.",
    U"Verifique sua senha.",
    U"Confirme sua senha.",
    U"As senhas não coincidem.",
    U"Verifique a confirmação da senha.",
    U"Digite sua chave de licença.",
    U"A chave deve ter 16 caracteres.",
    U"A chave só aceita letras e números.",
    U"Verifique sua chave de licença.",
    U"Usuário ou senha incorretos.",
    U"Conta bloqueada. Tente novamente mais tarde.",
    U"Sem conexão com o servidor. Verifique sua rede.",
    U"Esta chave de licença não é válida.",
    U"Este nome de usuário já está em uso.",
    U"Algo deu errado.",
    U"Counter-Strike 2 é o jogo de tiro tático em equipe da Valve. Plante ou desarme a bomba, domine a "
    U"economia e supere seus adversários em rodadas competitivas rápidas.",
    U"Escape from Tarkov é um jogo de tiro de extração hardcore. Explore mapas disputados, colete "
    U"equipamentos e saia vivo antes que o tempo da incursão acabe.",
    U"Fortnite é o battle royale gratuito da Epic Games. Salte com mais 99 jogadores, construa para "
    U"sobreviver e seja o último de pé.",
    U"Apex Legends é um jogo de tiro de heróis frenético da Respawn. Escolha uma Lenda com habilidades "
    U"únicas, forme um esquadrão e lute pelo título de campeão.",
    U"DayZ é um sandbox de sobrevivência em mundo aberto da Bohemia Interactive. Procure suprimentos e "
    U"sobreviva entre infectados e outros sobreviventes.",
    U"Tipo de injeção",
    U"Última atualização",
    U"Última detecção",
    U"Interno",
    U"Externo",
    U"nunca",
    U"Sua chave de licença",
    U"Ativar",
    U"Chave aceita.",
    U"Carregar",
    U"Licença",
    U"1 dia",
    U"1 semana",
    U"Vitalícia",
    U"Rust é um jogo de sobrevivência multiplayer implacável. Colete recursos, construa uma base e enfrente "
    U"outros jogadores em uma ilha hostil.",
    U"PUBG: Battlegrounds é um battle royale em grande escala. Salte de paraquedas com mais 99 jogadores, "
    U"encontre armas e sobreviva até o fim.",
    U"Rainbow Six Siege é um jogo de tiro tático focado em destruição e planejamento. Invada, defenda e "
    U"supere a equipe rival como um operador de elite.",
    U"Call of Duty traz ação militar explosiva, multiplayer acelerado e grandes batalhas no Warzone. "
    U"Equipe-se e entre na partida.",
    U"Grand Theft Auto V é uma aventura de ação em mundo aberto em Los Santos. Jogue com três criminosos em "
    U"grandes assaltos ou entre no caos online.",
};

constexpr Table kTurkish = {
    U"Tekrar hoş geldin",
    U"loader.gg'ye devam etmek için giriş yap",
    U"Kullanıcı adı",
    U"Şifre",
    U"Şifreyi onayla",
    U"Beni hatırla",
    U"Hesap oluştur",
    U"Giriş yap",
    U"Hesap oluşturuldu. Devam etmek için giriş yap.",
    U"Hesabını oluştur",
    U"Bir kullanıcı adı ve şifre seç",
    U"Girişe dön",
    U"Tekrar hoş geldin",
    U"Deneyime hazır mısın?",
    U"Çalışma alanın hazırlanıyor",
    U"Ürünlerin eşitleniyor",
    U"Neredeyse hazır",
    U"Ürünlerimiz",
    U"Kullanıcı adını gir.",
    U"Kullanıcı adı en az 3 karakter olmalı.",
    U"Kullanıcı adı en fazla 32 karakter olabilir.",
    U"Yalnızca harf, rakam, nokta, tire ve _ kullan.",
    U"Kullanıcı adını kontrol et.",
    U"Şifreni gir.",
    U"Şifre en az 6 karakter olmalı.",
    U"Şifre en fazla 128 karakter olabilir.",
    U"Şifreni kontrol et.",
    U"Şifreni onayla.",
    U"Şifreler eşleşmiyor.",
    U"Şifre onayını kontrol et.",
    U"Lisans anahtarını gir.",
    U"Anahtar 16 karakter olmalı.",
    U"Anahtar yalnızca harf ve rakam içerebilir.",
    U"Lisans anahtarını kontrol et.",
    U"Kullanıcı adı veya şifre hatalı.",
    U"Hesap kilitli. Daha sonra tekrar dene.",
    U"Sunucuya ulaşılamıyor. Bağlantını kontrol et.",
    U"Bu lisans anahtarı geçerli değil.",
    U"Bu kullanıcı adı zaten alınmış.",
    U"Bir şeyler ters gitti.",
    U"Counter-Strike 2, Valve'in taktik takım nişancı oyunudur. Bombayı kur veya etkisiz hale getir, "
    U"ekonomiyi yönet ve hızlı rekabetçi turlarda rakiplerini alt et.",
    U"Escape from Tarkov, zorlu bir tahliye nişancı oyunudur. Çekişmeli haritalara gir, ekipman topla ve "
    U"baskın süresi dolmadan sağ çık.",
    U"Fortnite, Epic Games'in ücretsiz battle royale oyunudur. 99 oyuncuyla birlikte atla, hayatta kalmak "
    U"için inşa et ve ayakta kalan son kişi ol.",
    U"Apex Legends, Respawn'ın hızlı tempolu kahraman nişancı oyunudur. Eşsiz yeteneklere sahip bir Efsane "
    U"seç, ekip kur ve şampiyonluk için savaş.",
    U"DayZ, Bohemia Interactive'in açık dünya hayatta kalma oyunudur. Malzeme ara ve enfekte olanlar ile "
    U"diğer hayatta kalanlar arasında ayakta kal.",
    U"Enjeksiyon türü",
    U"Son güncelleme",
    U"Son tespit",
    U"Dahili",
    U"Harici",
    U"hiç",
    U"Lisans anahtarını gir",
    U"Etkinleştir",
    U"Anahtar kabul edildi.",
    U"Yükle",
    U"Lisans",
    U"1 gün",
    U"1 hafta",
    U"Ömür boyu",
    U"Rust, acımasız bir çok oyunculu hayatta kalma oyunudur. Kaynak topla, üs kur ve düşmanca bir adada "
    U"diğer oyuncularla savaş.",
    U"PUBG: Battlegrounds, büyük ölçekli bir battle royale oyunudur. 99 oyuncuyla paraşütle atla, silah bul "
    U"ve sonuna kadar hayatta kal.",
    U"Rainbow Six Siege, yıkım ve planlama odaklı taktik bir takım nişancı oyunudur. Elit bir operatör "
    U"olarak saldır, savun ve rakip takımı alt et.",
    U"Call of Duty, patlayıcı askeri aksiyon, hızlı çok oyunculu modlar ve büyük Warzone savaşları sunar. "
    U"Hazırlan ve oyuna katıl.",
    U"Grand Theft Auto V, Los Santos'ta geçen açık dünya aksiyon macerasıdır. Üç suçluyla büyük soygunlar "
    U"yap ya da kaosu çevrimiçi yaşa.",
};

constexpr Table kPolish = {
    U"Witaj ponownie",
    U"Zaloguj się, aby kontynuować w loader.gg",
    U"Nazwa użytkownika",
    U"Hasło",
    U"Potwierdź hasło",
    U"Zapamiętaj mnie",
    U"Utwórz konto",
    U"Zaloguj się",
    U"Konto utworzone. Zaloguj się, aby kontynuować.",
    U"Utwórz konto",
    U"Wybierz nazwę użytkownika i hasło",
    U"Wróć do logowania",
    U"Witaj ponownie",
    U"Gotowy na to doświadczenie?",
    U"Przygotowanie przestrzeni",
    U"Synchronizacja produktów",
    U"Prawie gotowe",
    U"Nasze produkty",
    U"Wpisz nazwę użytkownika.",
    U"Nazwa użytkownika: min. 3 znaki.",
    U"Nazwa użytkownika: maks. 32 znaki.",
    U"Tylko litery, cyfry, kropki, myślniki i _.",
    U"Sprawdź nazwę użytkownika.",
    U"Wpisz hasło.",
    U"Hasło: min. 6 znaków.",
    U"Hasło: maks. 128 znaków.",
    U"Sprawdź hasło.",
    U"Potwierdź hasło.",
    U"Hasła nie są takie same.",
    U"Sprawdź potwierdzenie hasła.",
    U"Wpisz klucz licencyjny.",
    U"Klucz musi mieć 16 znaków.",
    U"Klucz może zawierać tylko litery i cyfry.",
    U"Sprawdź klucz licencyjny.",
    U"Nieprawidłowa nazwa użytkownika lub hasło.",
    U"Konto zablokowane. Spróbuj ponownie później.",
    U"Brak połączenia z serwerem. Sprawdź sieć.",
    U"Ten klucz licencyjny jest nieprawidłowy.",
    U"Ta nazwa użytkownika jest już zajęta.",
    U"Coś poszło nie tak.",
    U"Counter-Strike 2 to taktyczna strzelanka drużynowa od Valve. Podkładaj lub rozbrajaj bombę, opanuj "
    U"ekonomię i pokonuj rywali w szybkich, rankingowych rundach.",
    U"Escape from Tarkov to hardcorowa strzelanka z ekstrakcją. Przeszukuj sporne mapy, zbieraj sprzęt i "
    U"wydostań się żywy, zanim skończy się czas rajdu.",
    U"Fortnite to darmowe battle royale od Epic Games. Zeskocz z 99 innymi graczami, buduj, aby przetrwać, i "
    U"bądź ostatnim, który stoi.",
    U"Apex Legends to dynamiczna strzelanka z bohaterami od Respawn. Wybierz Legendę z unikalnymi "
    U"zdolnościami, stwórz drużynę i walcz o tytuł mistrza.",
    U"DayZ to otwartoświatowa piaskownica survivalowa od Bohemia Interactive. Szukaj zapasów i przetrwaj "
    U"wśród zarażonych oraz innych ocalałych.",
    U"Typ iniekcji",
    U"Ostatnia aktualizacja",
    U"Ostatnie wykrycie",
    U"Wewnętrzna",
    U"Zewnętrzna",
    U"nigdy",
    U"Wpisz klucz licencyjny",
    U"Aktywuj",
    U"Klucz zaakceptowany.",
    U"Załaduj",
    U"Licencja",
    U"1 dzień",
    U"1 tydzień",
    U"Dożywotnia",
    U"Rust to bezwzględna wieloosobowa gra survivalowa. Zbieraj zasoby, buduj bazę i walcz z innymi graczami "
    U"na wrogiej wyspie.",
    U"PUBG: Battlegrounds to wielkoskalowe battle royale. Wyskocz ze spadochronem z 99 graczami, zbieraj "
    U"broń i przetrwaj do końca.",
    U"Rainbow Six Siege to taktyczna strzelanka drużynowa oparta na destrukcji i planowaniu. Atakuj, broń i "
    U"przechytrz wrogą drużynę jako elitarny operator.",
    U"Call of Duty to wybuchowa akcja militarna, szybki tryb wieloosobowy i wielkie bitwy w Warzone. Wyposaż "
    U"się i dołącz do gry.",
    U"Grand Theft Auto V to otwartoświatowa gra akcji w Los Santos. Wciel się w trzech przestępców "
    U"planujących napady lub wejdź w chaos online.",
};

constexpr Table kItalian = {
    U"Bentornato",
    U"Accedi per continuare su loader.gg",
    U"Nome utente",
    U"Password",
    U"Conferma password",
    U"Ricordami",
    U"Crea account",
    U"Accedi",
    U"Account creato. Accedi per continuare.",
    U"Crea il tuo account",
    U"Scegli un nome utente e una password",
    U"Torna all'accesso",
    U"Bentornato",
    U"Pronto per l'esperienza?",
    U"Preparazione dello spazio di lavoro",
    U"Sincronizzazione dei prodotti",
    U"Quasi pronto",
    U"I nostri prodotti",
    U"Inserisci il nome utente.",
    U"Nome utente: almeno 3 caratteri.",
    U"Nome utente: massimo 32 caratteri.",
    U"Solo lettere, numeri, punti, trattini e _.",
    U"Controlla il nome utente.",
    U"Inserisci la password.",
    U"Password: almeno 6 caratteri.",
    U"Password: massimo 128 caratteri.",
    U"Controlla la password.",
    U"Conferma la password.",
    U"Le password non coincidono.",
    U"Controlla la conferma della password.",
    U"Inserisci la chiave di licenza.",
    U"La chiave deve avere 16 caratteri.",
    U"La chiave accetta solo lettere e numeri.",
    U"Controlla la chiave di licenza.",
    U"Nome utente o password errati.",
    U"Account bloccato. Riprova più tardi.",
    U"Server non raggiungibile. Controlla la connessione.",
    U"Questa chiave di licenza non è valida.",
    U"Questo nome utente è già in uso.",
    U"Qualcosa è andato storto.",
    U"Counter-Strike 2 è lo sparatutto tattico a squadre di Valve. Pianta o disinnesca la bomba, padroneggia "
    U"l'economia e batti gli avversari in round competitivi veloci.",
    U"Escape from Tarkov è uno sparatutto di estrazione hardcore. Esplora mappe contese, raccogli "
    U"equipaggiamento ed esci vivo prima che scada il tempo del raid.",
    U"Fortnite è il battle royale gratuito di Epic Games. Lanciati con altri 99 giocatori, costruisci per "
    U"sopravvivere e sii l'ultimo in piedi.",
    U"Apex Legends è uno sparatutto a eroi frenetico di Respawn. Scegli una Leggenda con abilità uniche, "
    U"forma una squadra e lotta per diventare campione.",
    U"DayZ è un sandbox di sopravvivenza open world di Bohemia Interactive. Cerca provviste e sopravvivi tra "
    U"infetti e altri sopravvissuti.",
    U"Tipo di iniezione",
    U"Ultimo aggiornamento",
    U"Ultimo rilevamento",
    U"Interno",
    U"Esterno",
    U"mai",
    U"Chiave di licenza",
    U"Attiva",
    U"Chiave accettata.",
    U"Carica",
    U"Licenza",
    U"1 giorno",
    U"1 settimana",
    U"A vita",
    U"Rust è un brutale gioco di sopravvivenza multiplayer. Raccogli risorse, costruisci una base e combatti "
    U"contro altri giocatori su un'isola ostile.",
    U"PUBG: Battlegrounds è un battle royale su larga scala. Paracaduti con altri 99 giocatori, trova armi e "
    U"resisti fino alla fine.",
    U"Rainbow Six Siege è uno sparatutto tattico a squadre basato su distruzione e pianificazione. Attacca, "
    U"difendi e supera la squadra avversaria come operatore d'élite.",
    U"Call of Duty offre azione militare esplosiva, multiplayer veloce e grandi battaglie in Warzone. "
    U"Equipaggiati e unisciti alla partita.",
    U"Grand Theft Auto V è un'avventura d'azione open world a Los Santos. Gioca con tre criminali impegnati "
    U"in colpi audaci o immergiti nel caos online.",
};

const Table& tableFor(Language language)
{
    switch (language)
    {
    case Language::German:
        return kGerman;
    case Language::Spanish:
        return kSpanish;
    case Language::Russian:
        return kRussian;
    case Language::French:
        return kFrench;
    case Language::Portuguese:
        return kPortuguese;
    case Language::Turkish:
        return kTurkish;
    case Language::Polish:
        return kPolish;
    case Language::Italian:
        return kItalian;
    case Language::English:
        break;
    }
    return kEnglish;
}

}

std::u32string_view languageName(Language language)
{
    switch (language)
    {
    case Language::German:
        return U"Deutsch";
    case Language::Spanish:
        return U"Español";
    case Language::Russian:
        return U"Русский";
    case Language::French:
        return U"Français";
    case Language::Portuguese:
        return U"Português";
    case Language::Turkish:
        return U"Türkçe";
    case Language::Polish:
        return U"Polski";
    case Language::Italian:
        return U"Italiano";
    case Language::English:
        break;
    }
    return U"English";
}

std::string_view languageCode(Language language)
{
    switch (language)
    {
    case Language::German:
        return "de";
    case Language::Spanish:
        return "es";
    case Language::Russian:
        return "ru";
    case Language::French:
        return "fr";
    case Language::Portuguese:
        return "pt";
    case Language::Turkish:
        return "tr";
    case Language::Polish:
        return "pl";
    case Language::Italian:
        return "it";
    case Language::English:
        break;
    }
    return "en";
}

std::optional<Language> languageFromCode(std::string_view code)
{
    for (const Language language : kLanguages)
    {
        if (languageCode(language) == code)
            return language;
    }
    return std::nullopt;
}

std::u32string_view translate(Language language, Str id)
{
    const char32_t* text = tableFor(language)[static_cast<std::size_t>(id)];
    return text != nullptr ? std::u32string_view(text) : std::u32string_view();
}

void Localization::setLanguage(Language language)
{
    if (language_ == language)
        return;
    language_ = language;

    const auto snapshot = listeners_;
    for (const auto& [id, listener] : snapshot)
    {
        const bool alive =
            std::ranges::any_of(listeners_, [id](const auto& entry) { return entry.first == id; });
        if (alive)
            listener();
    }
}

Localization::Subscription Localization::subscribe(std::function<void()> listener)
{
    const std::size_t id = nextId_++;
    listeners_.emplace_back(id, std::move(listener));
    return Subscription(*this, id);
}

void Localization::unsubscribe(std::size_t id)
{
    std::erase_if(listeners_, [id](const auto& entry) { return entry.first == id; });
}

}
