#pragma once

#ifdef COLOR_HEADER_C
// Reset
# define Color_Off          "\033[0m"             // Text Reset
# define ResetColor         "\033[0m"             // Text Reset

// Regular Colors
# define Black              "\033[0;30m"          // Black
# define Red                "\033[0;31m"          // Red
# define Green              "\033[0;32m"          // Green
# define Yellow             "\033[0;33m"          // Yellow
# define Blue               "\033[0;34m"          // Blue
# define Purple             "\033[0;35m"          // Purple
# define Cyan               "\033[0;36m"          // Cyan
# define White              "\033[0;37m"          // White
# define Gray               "\033[0;90m"          // Gray
# define Orange             "\033[0;38;5;208m"    // Orange
# define Pink               "\033[0;38;5;206m"    // Pink
# define LightBlue          "\033[0;38;5;117m"    // Light Blue
# define LightGreen         "\033[0;38;5;120m"    // Light Green
# define LightYellow        "\033[0;38;5;228m"    // Light Yellow
# define LightPurple        "\033[0;38;5;141m"    // Light Purple
# define LightCyan          "\033[0;38;5;123m"    // Light Cyan
# define LightGray          "\033[0;38;5;250m"    // Light Gray

// Bold
# define BBlack             "\033[1;30m"          // Bold Black
# define BRed               "\033[1;31m"          // Bold Red
# define BGreen             "\033[1;32m"          // Bold Green
# define BYellow            "\033[1;33m"          // Bold Yellow
# define BBlue              "\033[1;34m"          // Bold Blue
# define BPurple            "\033[1;35m"          // Bold Purple
# define BCyan              "\033[1;36m"          // Bold Cyan
# define BWhite             "\033[1;37m"          // Bold White
# define BGray              "\033[1;90m"          // Bold Gray
# define BOrange            "\033[1;38;5;208m"    // Bold Orange
# define BPink              "\033[1;38;5;206m"    // Bold Pink
# define BLightBlue         "\033[1;38;5;117m"    // Bold Light Blue
# define BLightGreen        "\033[1;38;5;120m"    // Bold Light Green
# define BLightYellow       "\033[1;38;5;228m"    // Bold Light Yellow
# define BLightPurple       "\033[1;38;5;141m"    // Bold Light Purple
# define BLightCyan         "\033[1;38;5;123m"    // Bold Light Cyan
# define BLightGray         "\033[1;38;5;250m"    // Bold Light Gray

// Underline
# define UBlack             "\033[4;30m"          // Underline Black
# define URed               "\033[4;31m"          // Underline Red
# define UGreen             "\033[4;32m"          // Underline Green
# define UYellow            "\033[4;33m"          // Underline Yellow
# define UBlue              "\033[4;34m"          // Underline Blue
# define UPurple            "\033[4;35m"          // Underline Purple
# define UCyan              "\033[4;36m"          // Underline Cyan
# define UWhite             "\033[4;37m"          // Underline White
# define UGray              "\033[4;90m"          // Underline Gray
# define UOrange            "\033[4;38;5;208m"    // Underline Orange
# define UPink              "\033[4;38;5;206m"    // Underline Pink
# define ULightBlue         "\033[4;38;5;117m"    // Underline Light Blue
# define ULightGreen        "\033[4;38;5;120m"    // Underline Light Green
# define ULightYellow       "\033[4;38;5;228m"    // Underline Light Yellow
# define ULightPurple       "\033[4;38;5;141m"    // Underline Light Purple
# define ULightCyan         "\033[4;38;5;123m"    // Underline Light Cyan
# define ULightGray         "\033[4;38;5;250m"    // Underline Light Gray

// Background
# define On_Black           "\033[40m"            // Background Black
# define On_Red             "\033[41m"            // Background Red
# define On_Green           "\033[42m"            // Background Green
# define On_Yellow          "\033[43m"            // Background Yellow
# define On_Blue            "\033[44m"            // Background Blue
# define On_Purple          "\033[45m"            // Background Purple
# define On_Cyan            "\033[46m"            // Background Cyan
# define On_White           "\033[47m"            // Background White
# define On_Gray            "\033[100m"           // Background Gray
# define On_Orange          "\033[48;5;208m"      // Background Orange
# define On_Pink            "\033[48;5;206m"      // Background Pink
# define On_LightBlue       "\033[48;5;117m"      // Background Light Blue
# define On_LightGreen      "\033[48;5;120m"      // Background Light Green
# define On_LightYellow     "\033[48;5;228m"      // Background Light Yellow
# define On_LightPurple     "\033[48;5;141m"      // Background Light Purple
# define On_LightCyan       "\033[48;5;123m"      // Background Light Cyan
# define On_LightGray       "\033[48;5;250m"      // Background Light Gray

// High Intensity
# define IBlack             "\033[0;90m"          // High Intensity Black
# define IRed               "\033[0;91m"          // High Intensity Red
# define IGreen             "\033[0;92m"          // High Intensity Green
# define IYellow            "\033[0;93m"          // High Intensity Yellow
# define IBlue              "\033[0;94m"          // High Intensity Blue
# define IPurple            "\033[0;95m"          // High Intensity Purple
# define ICyan              "\033[0;96m"          // High Intensity Cyan
# define IWhite             "\033[0;97m"          // High Intensity White
# define IGray              "\033[0;90m"          // High Intensity Gray
# define IOrange            "\033[0;38;5;208m"    // High Intensity Orange
# define IPink              "\033[0;38;5;206m"    // High Intensity Pink
# define ILightBlue         "\033[0;38;5;117m"    // High Intensity Light Blue
# define ILightGreen        "\033[0;38;5;120m"    // High Intensity Light Green
# define ILightYellow       "\033[0;38;5;228m"    // High Intensity Light Yellow
# define ILightPurple       "\033[0;38;5;141m"    // High Intensity Light Purple
# define ILightCyan         "\033[0;38;5;123m"    // High Intensity Light Cyan
# define ILightGray         "\033[0;38;5;250m"    // High Intensity Light Gray

// Bold High Intensity
# define BIBlack            "\033[1;90m"          // Bold High Intensity Black
# define BIRed              "\033[1;91m"          // Bold High Intensity Red
# define BIGreen            "\033[1;92m"          // Bold High Intensity Green
# define BIYellow           "\033[1;93m"          // Bold High Intensity Yellow
# define BIBlue             "\033[1;94m"          // Bold High Intensity Blue
# define BIPurple           "\033[1;95m"          // Bold High Intensity Purple
# define BICyan             "\033[1;96m"          // Bold High Intensity Cyan
# define BIWhite            "\033[1;97m"          // Bold High Intensity White
# define BIGray             "\033[1;90m"          // Bold High Intensity Gray
# define BIOrange           "\033[1;38;5;208m"    // Bold High Intensity Orange
# define BIPink             "\033[1;38;5;206m"    // Bold High Intensity Pink
# define BILightBlue        "\033[1;38;5;117m"    // Bold High Intensity Light Blue
# define BILightGreen       "\033[1;38;5;120m"    // Bold High Intensity Light Green
# define BILightYellow      "\033[1;38;5;228m"    // Bold High Intensity Light Yellow
# define BILightPurple      "\033[1;38;5;141m"    // Bold High Intensity Light Purple
# define BILightCyan        "\033[1;38;5;123m"    // Bold High Intensity Light Cyan
# define BILightGray        "\033[1;38;5;250m"    // Bold High Intensity Light Gray

// Backgrounds
# define On_IBlack          "\033[0;100m"         // Backgrounds Black
# define On_IRed            "\033[0;101m"         // Backgrounds Red
# define On_IGreen          "\033[0;102m"         // Backgrounds Green
# define On_IYellow         "\033[0;103m"         // Backgrounds Yellow
# define On_IBlue           "\033[0;104m"         // Backgrounds Blue
# define On_IPurple         "\033[0;105m"         // Backgrounds Purple
# define On_ICyan           "\033[0;106m"         // Backgrounds Cyan
# define On_IWhite          "\033[0;107m"         // Backgrounds White
# define On_IGray           "\033[0;100m"         // Backgrounds Gray
# define On_IOrange         "\033[0;48;5;208m"    // Backgrounds Orange
# define On_IPink           "\033[0;48;5;206m"    // Backgrounds Pink
# define On_ILightBlue      "\033[0;48;5;117m"    // Backgrounds Light Blue
# define On_ILightGreen     "\033[0;48;5;120m"    // Backgrounds Light Green
# define On_ILightYellow    "\033[0;48;5;228m"    // Backgrounds Light Yellow
# define On_ILightPurple    "\033[0;48;5;141m"    // Backgrounds Light Purple
# define On_ILightCyan      "\033[0;48;5;123m"    // Backgrounds Light Cyan
# define On_ILightGray      "\033[0;48;5;250m"    // Backgrounds Light Gray

#endif

#ifdef COLOR_HEADER_CXX
# include <string>
// Reset
# define Color_Off          (std::string)"\033[0m"             // Text Reset
# define ResetColor         (std::string)"\033[0m"             // Text Reset

// Regular Colors
# define Black              (std::string)"\033[0;30m"          // Black
# define Red                (std::string)"\033[0;31m"          // Red
# define Green              (std::string)"\033[0;32m"          // Green
# define Yellow             (std::string)"\033[0;33m"          // Yellow
# define Blue               (std::string)"\033[0;34m"          // Blue
# define Purple             (std::string)"\033[0;35m"          // Purple
# define Cyan               (std::string)"\033[0;36m"          // Cyan
# define White              (std::string)"\033[0;37m"          // White
# define Gray               (std::string)"\033[0;90m"          // Gray
# define Orange             (std::string)"\033[0;38;5;208m"    // Orange
# define Pink               (std::string)"\033[0;38;5;206m"    // Pink
# define LightBlue          (std::string)"\033[0;38;5;117m"    // Light Blue
# define LightGreen         (std::string)"\033[0;38;5;120m"    // Light Green
# define LightYellow        (std::string)"\033[0;38;5;228m"    // Light Yellow
# define LightPurple        (std::string)"\033[0;38;5;141m"    // Light Purple
# define LightCyan          (std::string)"\033[0;38;5;123m"    // Light Cyan
# define LightGray          (std::string)"\033[0;38;5;250m"    // Light Gray

// Bold
# define BBlack             (std::string)"\033[1;30m"          // Bold Black
# define BRed               (std::string)"\033[1;31m"          // Bold Red
# define BGreen             (std::string)"\033[1;32m"          // Bold Green
# define BYellow            (std::string)"\033[1;33m"          // Bold Yellow
# define BBlue              (std::string)"\033[1;34m"          // Bold Blue
# define BPurple            (std::string)"\033[1;35m"          // Bold Purple
# define BCyan              (std::string)"\033[1;36m"          // Bold Cyan
# define BWhite             (std::string)"\033[1;37m"          // Bold White
# define BGray              (std::string)"\033[1;90m"          // Bold Gray
# define BOrange            (std::string)"\033[1;38;5;208m"    // Bold Orange
# define BPink              (std::string)"\033[1;38;5;206m"    // Bold Pink
# define BLightBlue         (std::string)"\033[1;38;5;117m"    // Bold Light Blue
# define BLightGreen        (std::string)"\033[1;38;5;120m"    // Bold Light Green
# define BLightYellow       (std::string)"\033[1;38;5;228m"    // Bold Light Yellow
# define BLightPurple       (std::string)"\033[1;38;5;141m"    // Bold Light Purple
# define BLightCyan         (std::string)"\033[1;38;5;123m"    // Bold Light Cyan
# define BLightGray         (std::string)"\033[1;38;5;250m"    // Bold Light Gray

// Underline
# define UBlack             (std::string)"\033[4;30m"          // Underline Black
# define URed               (std::string)"\033[4;31m"          // Underline Red
# define UGreen             (std::string)"\033[4;32m"          // Underline Green
# define UYellow            (std::string)"\033[4;33m"          // Underline Yellow
# define UBlue              (std::string)"\033[4;34m"          // Underline Blue
# define UPurple            (std::string)"\033[4;35m"          // Underline Purple
# define UCyan              (std::string)"\033[4;36m"          // Underline Cyan
# define UWhite             (std::string)"\033[4;37m"          // Underline White
# define UGray              (std::string)"\033[4;90m"          // Underline Gray
# define UOrange            (std::string)"\033[4;38;5;208m"    // Underline Orange
# define UPink              (std::string)"\033[4;38;5;206m"    // Underline Pink
# define ULightBlue         (std::string)"\033[4;38;5;117m"    // Underline Light Blue
# define ULightGreen        (std::string)"\033[4;38;5;120m"    // Underline Light Green
# define ULightYellow       (std::string)"\033[4;38;5;228m"    // Underline Light Yellow
# define ULightPurple       (std::string)"\033[4;38;5;141m"    // Underline Light Purple
# define ULightCyan         (std::string)"\033[4;38;5;123m"    // Underline Light Cyan
# define ULightGray         (std::string)"\033[4;38;5;250m"    // Underline Light Gray

// Background
# define On_Black           (std::string)"\033[40m"            // Background Black
# define On_Red             (std::string)"\033[41m"            // Background Red
# define On_Green           (std::string)"\033[42m"            // Background Green
# define On_Yellow          (std::string)"\033[43m"            // Background Yellow
# define On_Blue            (std::string)"\033[44m"            // Background Blue
# define On_Purple          (std::string)"\033[45m"            // Background Purple
# define On_Cyan            (std::string)"\033[46m"            // Background Cyan
# define On_White           (std::string)"\033[47m"            // Background White
# define On_Gray            (std::string)"\033[100m"           // Background Gray
# define On_Orange          (std::string)"\033[48;5;208m"      // Background Orange
# define On_Pink            (std::string)"\033[48;5;206m"      // Background Pink
# define On_LightBlue       (std::string)"\033[48;5;117m"      // Background Light Blue
# define On_LightGreen      (std::string)"\033[48;5;120m"      // Background Light Green
# define On_LightYellow     (std::string)"\033[48;5;228m"      // Background Light Yellow
# define On_LightPurple     (std::string)"\033[48;5;141m"      // Background Light Purple
# define On_LightCyan       (std::string)"\033[48;5;123m"      // Background Light Cyan
# define On_LightGray       (std::string)"\033[48;5;250m"      // Background Light Gray

// High Intensity
# define IBlack             (std::string)"\033[0;90m"          // High Intensity Black
# define IRed               (std::string)"\033[0;91m"          // High Intensity Red
# define IGreen             (std::string)"\033[0;92m"          // High Intensity Green
# define IYellow            (std::string)"\033[0;93m"          // High Intensity Yellow
# define IBlue              (std::string)"\033[0;94m"          // High Intensity Blue
# define IPurple            (std::string)"\033[0;95m"          // High Intensity Purple
# define ICyan              (std::string)"\033[0;96m"          // High Intensity Cyan
# define IWhite             (std::string)"\033[0;97m"          // High Intensity White
# define IGray              (std::string)"\033[0;90m"          // High Intensity Gray
# define IOrange            (std::string)"\033[0;38;5;208m"    // High Intensity Orange
# define IPink              (std::string)"\033[0;38;5;206m"    // High Intensity Pink
# define ILightBlue         (std::string)"\033[0;38;5;117m"    // High Intensity Light Blue
# define ILightGreen        (std::string)"\033[0;38;5;120m"    // High Intensity Light Green
# define ILightYellow       (std::string)"\033[0;38;5;228m"    // High Intensity Light Yellow
# define ILightPurple       (std::string)"\033[0;38;5;141m"    // High Intensity Light Purple
# define ILightCyan         (std::string)"\033[0;38;5;123m"    // High Intensity Light Cyan
# define ILightGray         (std::string)"\033[0;38;5;250m"    // High Intensity Light Gray

// Bold High Intensity
# define BIBlack            (std::string)"\033[1;90m"          // Bold High Intensity Black
# define BIRed              (std::string)"\033[1;91m"          // Bold High Intensity Red
# define BIGreen            (std::string)"\033[1;92m"          // Bold High Intensity Green
# define BIYellow           (std::string)"\033[1;93m"          // Bold High Intensity Yellow
# define BIBlue             (std::string)"\033[1;94m"          // Bold High Intensity Blue
# define BIPurple           (std::string)"\033[1;95m"          // Bold High Intensity Purple
# define BICyan             (std::string)"\033[1;96m"          // Bold High Intensity Cyan
# define BIWhite            (std::string)"\033[1;97m"          // Bold High Intensity White
# define BIGray             (std::string)"\033[1;90m"          // Bold High Intensity Gray
# define BIOrange           (std::string)"\033[1;38;5;208m"    // Bold High Intensity Orange
# define BIPink             (std::string)"\033[1;38;5;206m"    // Bold High Intensity Pink
# define BILightBlue        (std::string)"\033[1;38;5;117m"    // Bold High Intensity Light Blue
# define BILightGreen       (std::string)"\033[1;38;5;120m"    // Bold High Intensity Light Green
# define BILightYellow      (std::string)"\033[1;38;5;228m"    // Bold High Intensity Light Yellow
# define BILightPurple      (std::string)"\033[1;38;5;141m"    // Bold High Intensity Light Purple
# define BILightCyan        (std::string)"\033[1;38;5;123m"    // Bold High Intensity Light Cyan
# define BILightGray        (std::string)"\033[1;38;5;250m"    // Bold High Intensity Light Gray

// Backgrounds
# define On_IBlack          (std::string)"\033[0;100m"         // Backgrounds Black
# define On_IRed            (std::string)"\033[0;101m"         // Backgrounds Red
# define On_IGreen          (std::string)"\033[0;102m"         // Backgrounds Green
# define On_IYellow         (std::string)"\033[0;103m"         // Backgrounds Yellow
# define On_IBlue           (std::string)"\033[0;104m"         // Backgrounds Blue
# define On_IPurple         (std::string)"\033[0;105m"         // Backgrounds Purple
# define On_ICyan           (std::string)"\033[0;106m"         // Backgrounds Cyan
# define On_IWhite          (std::string)"\033[0;107m"         // Backgrounds White
# define On_IGray           (std::string)"\033[0;100m"         // Backgrounds Gray
# define On_IOrange         (std::string)"\033[0;48;5;208m"    // Backgrounds Orange
# define On_IPink           (std::string)"\033[0;48;5;206m"    // Backgrounds Pink
# define On_ILightBlue      (std::string)"\033[0;48;5;117m"    // Backgrounds Light Blue
# define On_ILightGreen     (std::string)"\033[0;48;5;120m"    // Backgrounds Light Green
# define On_ILightYellow    (std::string)"\033[0;48;5;228m"    // Backgrounds Light Yellow
# define On_ILightPurple    (std::string)"\033[0;48;5;141m"    // Backgrounds Light Purple
# define On_ILightCyan      (std::string)"\033[0;48;5;123m"    // Backgrounds Light Cyan
# define On_ILightGray      (std::string)"\033[0;48;5;250m"    // Backgrounds Light Gray
#endif