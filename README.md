# enetproxy
A Modified &amp; Fixed Enet proxy (Source by ama6nen) for Private Servers 
- Owner of source : https://github.com/ama6nen

# Modified Features
- Fixed Cannot join private servers because of Real Growtopia peer check.
- Added config.json for simple usage.
- Added /follow command. to use, wrenchset netid and wrenchmode. Then wrench the player you want to stick and follow.
- lagspike for some servers without packet limit check
- In-Game packet tracing
- To send an action use the following command /action <action> .
- /send command to send the saved action.
- /wrenchset netid/trade/pull/ban/kick
- /wrenchmode to enable wrench mode
- you can tp to player with /tp command if you already set a netid by wrenching player.

# NOTE: Replace "|" with "." and "\n" with " " while setting up an action.
- For Example : "/action action.dialog_return dialog_name.editsign text.proxy"
- And packet will be like:
- "action|dialog_return\ndialog_name|editsign\ntext|proxy"
