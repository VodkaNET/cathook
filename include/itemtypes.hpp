/*
 * itemtypes.h
 *
 *  Created on: Feb 10, 2017
 *      Author: nullifiedcat
 */

#ifndef ITEMTYPES_HPP_
#define ITEMTYPES_HPP_

#include <aftercheaders.hpp>
#include <beforecheaders.hpp>
#include <map>
#include <string>
#include <vector>

const std::string tf2c_weapon_names[] = { "BOTTLE",
                                          "GRENADE_NAPALM",
                                          "SUPERSHOTGUN_MERCENARY",
                                          "ROCKETBETA",
                                          "GRENADE_GRENADELAUNCHER",
                                          "GRENADELAUNCHER",
                                          "GRENADE_PIPEBOMB",
                                          "CIGARETTE_CASE",
                                          "BRANDINGIRON",
                                          "BANHAMMER",
                                          "SNIPERRIFLE",
                                          "GRENADE_HEAL",
                                          "HAMMERFISTS",
                                          "GRENADE_NAIL",
                                          "DART",
                                          "RPG",
                                          "UMBRELLA_CIVILIAN",
                                          "GRENADE_FRAG",
                                          "GRENADE_MIRV",
                                          "MEDIGUN",
                                          "GRENADE_MIRV_DEMO",
                                          "PICKAXE",
                                          "SYRINGEGUN",
                                          "SCATTERGUN",
                                          "STENGUN",
                                          "SNUBNOSE",
                                          "MINIGUN",
                                          "WRENCH",
                                          "BAT",
                                          "SAPPER",
                                          "ROCKET",
                                          "HAMMER",
                                          "FISHWHACKER",
                                          "KRITZKRIEG",
                                          "FLAREGUN",
                                          "SHOVEL",
                                          "PISTOL",
                                          "SMG",
                                          "LEADPIPE",
                                          "FLAREGUN_SHELL",
                                          "GRENADE_EMP",
                                          "CYCLOPS",
                                          "MACHETE",
                                          "SHOTGUN",
                                          "REVOLVER",
                                          "GRENADE_GAS",
                                          "GRENADE_BEARTRAP",
                                          "FIREAXE",
                                          "GRENADE_CONC",
                                          "NAILGUN",
                                          "BONESAW",
                                          "STICKYBOMB_LAUNCHER",
                                          "GRENADE_BOMBLET",
                                          "TRANQ",
                                          "DYNAMITE",
                                          "CLUB",
                                          "TOMMYGUN",
                                          "COFFEPOT",
                                          "STICKYBOMB",
                                          "ROCKETLAUNCHER",
                                          "FLAMETHROWER",
                                          "CROWBAR",
                                          "BUILDER",
                                          "HEAVY_ARTILLERY",
                                          "KNIFE",
                                          "PDA_ENGINEER",
                                          "BOTTLE_BROKEN",
                                          "TOOLBOX",
                                          "SYRINGE_PROJ",
                                          "NAIL",
                                          "SYRINGE",
                                          "OVERHEALER" };

enum k_EItemType
{
    ITEM_NONE = 0,

    ITEM_HEALTH_SMALL,
    ITEM_HEALTH_MEDIUM,
    ITEM_HEALTH_LARGE,

    ITEM_AMMO_SMALL,
    ITEM_AMMO_MEDIUM,
    ITEM_AMMO_LARGE,

    ITEM_POWERUP_STRENGTH,
    ITEM_POWERUP_RESISTANCE,
    ITEM_POWERUP_VAMPIRE,
    ITEM_POWERUP_REFLECT,
    ITEM_POWERUP_HASTE,
    ITEM_POWERUP_REGENERATION,
    ITEM_POWERUP_PRECISION,
    ITEM_POWERUP_AGILITY,
    ITEM_POWERUP_KNOCKOUT,
    ITEM_POWERUP_KING,
    ITEM_POWERUP_PLAGUE,
    ITEM_POWERUP_SUPERNOVA,
    ITEM_POWERUP_CRITS,

    ITEM_POWERUP_FIRST = ITEM_POWERUP_STRENGTH,
    ITEM_POWERUP_LAST  = ITEM_POWERUP_CRITS,

    ITEM_TF2C_PILL,
    ITEM_TF2C_CRITS,

    ITEM_TF2C_W_BOTTLE,
    ITEM_TF2C_W_GRENADE_NAPALM,
    ITEM_TF2C_W_SUPERSHOTGUN_MERCENARY,
    ITEM_TF2C_W_ROCKETBETA,
    ITEM_TF2C_W_GRENADE_GRENADELAUNCHER,
    ITEM_TF2C_W_GRENADELAUNCHER,
    ITEM_TF2C_W_GRENADE_PIPEBOMB,
    ITEM_TF2C_W_CIGARETTE_CASE,
    ITEM_TF2C_W_BRANDINGIRON,
    ITEM_TF2C_W_BANHAMMER,
    ITEM_TF2C_W_SNIPERRIFLE,
    ITEM_TF2C_W_GRENADE_HEAL,
    ITEM_TF2C_W_HAMMERFISTS,
    ITEM_TF2C_W_GRENADE_NAIL,
    ITEM_TF2C_W_DART,
    ITEM_TF2C_W_RPG,
    ITEM_TF2C_W_UMBRELLA_CIVILIAN,
    ITEM_TF2C_W_GRENADE_FRAG,
    ITEM_TF2C_W_GRENADE_MIRV,
    ITEM_TF2C_W_MEDIGUN,
    ITEM_TF2C_W_GRENADE_MIRV_DEMO,
    ITEM_TF2C_W_PICKAXE,
    ITEM_TF2C_W_SYRINGEGUN,
    ITEM_TF2C_W_SCATTERGUN,
    ITEM_TF2C_W_STENGUN,
    ITEM_TF2C_W_SNUBNOSE,
    ITEM_TF2C_W_MINIGUN,
    ITEM_TF2C_W_WRENCH,
    ITEM_TF2C_W_BAT,
    ITEM_TF2C_W_SAPPER,
    ITEM_TF2C_W_ROCKET,
    ITEM_TF2C_W_HAMMER,
    ITEM_TF2C_W_FISHWHACKER,
    ITEM_TF2C_W_KRITZKRIEG,
    ITEM_TF2C_W_FLAREGUN,
    ITEM_TF2C_W_SHOVEL,
    ITEM_TF2C_W_PISTOL,
    ITEM_TF2C_W_SMG,
    ITEM_TF2C_W_LEADPIPE,
    ITEM_TF2C_W_FLAREGUN_SHELL,
    ITEM_TF2C_W_GRENADE_EMP,
    ITEM_TF2C_W_CYCLOPS,
    ITEM_TF2C_W_MACHETE,
    ITEM_TF2C_W_SHOTGUN,
    ITEM_TF2C_W_REVOLVER,
    ITEM_TF2C_W_GRENADE_GAS,
    ITEM_TF2C_W_GRENADE_BEARTRAP,
    ITEM_TF2C_W_FIREAXE,
    ITEM_TF2C_W_GRENADE_CONC,
    ITEM_TF2C_W_NAILGUN,
    ITEM_TF2C_W_BONESAW,
    ITEM_TF2C_W_STICKYBOMB_LAUNCHER,
    ITEM_TF2C_W_GRENADE_BOMBLET,
    ITEM_TF2C_W_TRANQ,
    ITEM_TF2C_W_DYNAMITE,
    ITEM_TF2C_W_CLUB,
    ITEM_TF2C_W_TOMMYGUN,
    ITEM_TF2C_W_COFFEPOT,
    ITEM_TF2C_W_STICKYBOMB,
    ITEM_TF2C_W_ROCKETLAUNCHER,
    ITEM_TF2C_W_FLAMETHROWER,
    ITEM_TF2C_W_CROWBAR,
    ITEM_TF2C_W_BUILDER,
    ITEM_TF2C_W_HEAVY_ARTILLERY,
    ITEM_TF2C_W_KNIFE,
    ITEM_TF2C_W_PDA_ENGINEER,
    ITEM_TF2C_W_BOTTLE_BROKEN,
    ITEM_TF2C_W_TOOLBOX,
    ITEM_TF2C_W_SYRINGE_PROJ,
    ITEM_TF2C_W_NAIL,
    ITEM_TF2C_W_SYRINGE,
    ITEM_TF2C_W_OVERHEALER,

    ITEM_TF2C_W_FIRST = ITEM_TF2C_W_BOTTLE,
    ITEM_TF2C_W_LAST  = ITEM_TF2C_W_OVERHEALER,

    ITEM_HL_BATTERY,

    ITEM_SPELL,
    ITEM_SPELL_RARE,

    ITEM_COUNT
};

class CachedEntity;
typedef bool (*ItemCheckerFn)(CachedEntity *);
typedef k_EItemType (*ItemSpecialMapperFn)(CachedEntity *);

class ItemModelMapper
{
public:
    void RegisterItem(std::string modelpath, k_EItemType type);
    k_EItemType GetItemType(CachedEntity *entity);

    std::map<std::string, k_EItemType> models;
    std::map<uintptr_t, k_EItemType> map;
};

class ItemManager
{
public:
    ItemManager();
    void RegisterModelMapping(std::string path, k_EItemType type);
    void RegisterSpecialMapping(ItemCheckerFn fn, k_EItemType type);
    k_EItemType GetItemType(CachedEntity *ent);

    std::map<ItemCheckerFn, k_EItemType> special_map;
    std::vector<ItemSpecialMapperFn> specials;
    ItemModelMapper mapper_special;
    ItemModelMapper mapper;
};

extern ItemManager g_ItemManager;

#endif /* ITEMTYPES_HPP_ */
