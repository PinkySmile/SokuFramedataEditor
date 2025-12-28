//
// Created by PinkySmile on 27/12/2025.
//

#ifndef SOFGV_OBJECT_UTILS_HPP
#define SOFGV_OBJECT_UTILS_HPP


#define OBJECT_CHECK_FIELD(cls, extra, ptr1, ptr2, field, str) do { if ((ptr1)->field != (ptr2)->field) game->logger.fatal(std::string(msgStart) + cls "::" extra #field ": " + str((ptr1)->field) + " vs " + str((ptr2)->field)); } while (false)
#define OBJECT_CHECK_FIELD_ARR(cls, extra, ptr1, ptr2, field, i, str) do { if ((ptr1)->field[i] != (ptr2)->field[i]) game->logger.fatal(std::string(msgStart) + cls "::" extra #field "[" + std::to_string(i) + "]: " + str((ptr1)->field[i]) + " vs " + str((ptr2)->field[i])); } while (false)
#define OBJECT_CHECK_FIELD_VAL(cls, val1, val2, field, str) do { if (val1 != val2) game->logger.fatal(std::string(msgStart) + cls "::" field ": " + str(val1) + " vs " + str(val2)); } while (false)
#define DISPLAY_FIELD(cls, extra, ptr, field, str) game->logger.info(std::string(msgStart) + cls "::" extra #field ": " + str((ptr)->field))
#define DISPLAY_FIELD_ARR(cls, extra, ptr, field, i, str) game->logger.info(std::string(msgStart) + cls "::" extra #field "[" + std::to_string(i) + "]: " + str((ptr)->field[i]))
#define DISPLAY_FIELD_VAL(cls, val, field, str) game->logger.info(std::string(msgStart) + cls "::" field ": " + str(val))
#define HEX2(n) Utils::toHex<unsigned char, 1>(n)
#define DISP_VEC(v)  ("(" + std::to_string((v).x)     + ", " + std::to_string((v).y)      + ")")
#define DISP_PAIR(v) ("{" + std::to_string((v).first) + ", " + std::to_string((v).second) + "}")
#define DISP_BOOL(v) ((v) ? "true" : "false")


#endif //SOFGV_OBJECT_UTILS_HPP