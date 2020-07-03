If the "epicid" entry is missing OR invalid, the emulator will generate one based on your "username".
If the "username" entry is missing OR invalid, it will be reset to the default: DefaultName.

Any entry with an invalid value will be reset to its default.

"disable_online_networking" is an option that will try to disable all non-lan networking. Making the game not being able to connects to any online servers.
It should work on Windows, Linux and Macos but it was only tested on Windows. If you use it and it causes a crash, try to set it to false.

catalog.json        : Catalog Item IDs that you'd like to own. See lines EOS_Ecom_QueryOwnership in the log for Catalog ids. If there is no catalog.json or a specific Catalog ID is missing, the emulator will try to own the Catalog ID if you've set unlock_dlcs to true
entitlements_db.json: Entitlements that are defined on Epic Store. It will contain the definitions of the entitlements. See lines QueryEntitlements in the log for Entitlement IDs.
entitlements.json   : Contains pair of {entitlement id - redeemed}. It will make you own a specific entitlement.
achievements_db.json: Contains all achievements definitions. See lines EOS_Achievements_CopyAchievementDefinition in the log for achievements details.
achievements.json   : Contains all achievements definitions for a player. See lines EOS_Achievements_CopyPlayerAchievement in the log for achievements details.