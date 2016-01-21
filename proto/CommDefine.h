#ifndef COMMDEFINE_H
#define COMMDEFINE_H
/**每个帐号在每个服最多10个角色**/
#define MAX_PLAYER_NUM  10
/**名字的最大长度**/
#define MAX_PLAYER_NAME_LEN  255
/**副本最大数目**/
#define MAX_RAID_NUM  600
/**获取副本房间列表的最大数目**/
#define MAX_RAID_LIST_NUM  100
/**背包最大数目**/
#define MAX_BAG_NUM  100
/**最大命格背包的大小**/
#define MAX_MINGGE_BAG_NUM  24
/**装备命格的数目**/
#define MAX_MINGGE_INSTALL_NUM  8
/**猎命命格的数目**/
#define MAX_LIEMINGGE_NUM  18
/**最大的命格等级**/
#define MAX_MINGGE_LEVEL  8
/**命格碎片ID**/
#define MINGGE_CHIP_ID  8001
/**命格价格**/
#define MINGGE_CHIP_PRICE  2000
/**任务的最大数目**/
#define MAX_QUEST_NUM  400
/**最多的同时进行的任务数目**/
#define MAX_UNDERWAY_QUEST_NUM  50
/**每个装备最多镶嵌的宝石数**/
#define MAX_DIAMOND_PER_EQUIP  5
/**每个邮件最多的附件数目**/
#define MAX_ATTACH_PER_MAIL  50
/**每个职业最多的技能数目**/
#define MAX_SKILL_NUM  50
/**战斗力**/
#define RANK_FIGHTINGCAPACITY_TYPE  0
/**竟技场playerid->排行**/
#define RANK_COMPETITIVE_PLAYER_TYPE  1
/**竟技场排行->playerid**/
#define RANK_COMPETITIVE_RANK_TYPE  2
/**竟技场历史战绩**/
#define RANK_COMPETITIVE_REOCRD_TYPE  3
/**争霸赛排行**/
#define RANK_TOURNAMENT_TYPE  4
/**社团排行**/
#define RANK_ASSOCIATION_TYPE  5
/**玩家等级排行**/
#define RANK_PLAYER_LEVEL_TYPE  6
/**马仔战争力排行**/
#define RANK_PLAYER_MACHAI_TYPE  7
/**闯东星排行**/
#define RANK_THOUSAND_TOWER_TYPE  8
/**命格排行**/
#define RANK_MINGGE_TYPE  9
/**翅膀排行**/
#define RANK_WING_TYPE  10
/**龙纹排行**/
#define RANK_DRAGON_TYPE  11
/**装备排行**/
#define RANK_EQUIP_TYPE  12
/**饰品排行**/
#define RANK_JEWELRY_TYPE  13
/**宝石排行**/
#define RANK_GEN_TYPE  14
/**称号排行**/
#define RANK_TITLE_TYPE  15
/**宠物排行**/
#define RANK_PET_TYPE  16
/**最大的时装数目**/
#define MAX_SHIZHUANG_NUM  6
/**最大的翅膀数目**/
#define MAX_WING_NUM  8
/**每日最大的组队副本数目**/
#define MAX_TEAM_RAID_DAY_NUM  3
/**每10分钟增加1个体力**/
#define MAX_ADD_POWER_TIME  360
/**以邮件id方式读取配置表获取邮件内容**/
#define MAIL_TYPE_BY_ID  1
/**邮件中包含了邮件内容**/
#define MAIL_TYPE_BY_CONTENT  2
/**最大马仔数目**/
#define MAX_MACHAI_NUM  512
/**最大竞技场历史战绩数目**/
#define MAX_ARENA_RECORD_NUM  10
/**最大龙纹数目**/
#define MAX_DRAGON_NUM  200
/**商店物品最大数目**/
#define MAX_SHOP_ITEM_NUM  6
/**竞技场商店物品最大数目**/
#define MAX_COMPETITIVE_SHOP_ITEM_NUM  12
/**最大的vip等级**/
#define MAX_VIP_LEVEL_NUM  15
/**小秘书推送优先级1**/
#define SECRETARY_NOTIFY_LEVEL1  1
/**小秘书推送优先级2**/
#define SECRETARY_NOTIFY_LEVEL2  2
/**小秘书推送优先级3**/
#define SECRETARY_NOTIFY_LEVEL3  3
/**小秘书推送优先级4**/
#define SECRETARY_NOTIFY_LEVEL4  4
/**小秘书推送优先级5**/
#define SECRETARY_NOTIFY_LEVEL5  5
/**小秘书推送优先级6**/
#define SECRETARY_NOTIFY_LEVEL6  6
/**小秘书推送优先级7**/
#define SECRETARY_NOTIFY_LEVEL7  7
/**小秘书推送优先级8**/
#define SECRETARY_NOTIFY_LEVEL8  8
/**小秘书推送升级技能**/
#define SECRETARY_SKILL_UPGRADE  1
/**小秘书推送解锁技能**/
#define SECRETARY_SKILL_UNLOCK  2
/**最大称号值**/
#define MAX_PLAYER_TITLE_VALUE  200
/**最大成就值**/
#define MAX_ACHIEVEMENT_VALUE  400
/**最大竟技场历史记录值**/
#define MAX_COMPETITIVE_HISTORY_VALUE  10
/**最大竟技场历史奖励值**/
#define MAX_COMPETITIVE_HISTORY_REWARD_VALUE  10
/**日常任务最大值**/
#define MAX_DAILY_QUEST_VALUE  60
/**最大马仔数量**/
#define MAX_MACHAI_VALUE  512
/**主线任务**/
#define QUEST_FLAG_MAIN  1
/**支线任务**/
#define QUEST_FLAG_FEEDER  2
/**技能配置数目**/
#define MAX_SKILL_CONFIG_NUM  4
/**最大朋友数**/
#define MAX_FRIEND_NUM  50
/**最大用户等级**/
#define MAX_PLAYER_LEVEL_NUM  100
/**最大副本章节**/
#define MAX_RAID_SECTION_NUM  40
/**每天最大赠送VIP经验值**/
#define MAX_VIP_EXP_DAILY_NUM  12
/**vip经验每10分钟增加1点**/
#define MAX_VIP_EXP_CHECK_TIME  600
/**好友每日赠送体力次数上限**/
#define MAX_FRIEND_SEND_POWER_NUM  15
/**好友每日接受体力次数上限**/
#define MAX_FRIEND_RECV_POWER_NUM  10
/**赠送体力值**/
#define MAX_SEND_POWER_VAL  3
/**拼战等待时间**/
#define FIGHT_WAIT_TIME  1800
/**拼战结算时间**/
#define FIGHT_RUN_TIME  600
/**拼战每天最大次数**/
#define FIGHT_MAX_COUNT  4
/**援护cd时间**/
#define HELPER_CD_TIME  72000
/**十年抽唯一出现的物品最大数量**/
#define MAX_TEN_ACT_ONLY_NUM  10
/**累计充值最大记录数目**/
#define MAX_TOTAL_RECHARGE_NUM  30
/**最大马仔限时物品数目**/
#define MAX_MACHAI_TIME_LIMIT_PROP_VAL  20
/**经验命格类型**/
#define MINGGE_EXP_TYPE  99
#endif