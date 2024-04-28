//
// Created by Gegel85 on 12/03/2022.
//

#ifndef BATTLE_VERSION_H
#define BATTLE_VERSION_H

#define REAL_VERSION_STR "alpha 0.2.3"
#ifdef _DEBUG
#define VERSION_STR REAL_VERSION_STR" DEBUG"
#else
#define VERSION_STR REAL_VERSION_STR
#endif

#endif //BATTLE_VERSION_H
