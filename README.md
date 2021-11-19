# enetproxy
A Modified &amp; Fixed Enet proxy (Source by ama6nen) for Private Servers

# Modified Features
- Fixed Cannot join private servers because of Real Growtopia peer check.
- Added config.json for simple usage.
- Added /follow command
- lagspike for some servers without packet limit check
- In-Game packet tracing
- To send an action replace "|" with "." and "\n" with " " for example:
"/action action.dialog_return dialog_name.editsign text.proxy"
and packet will be like:
"action|dialog_return\ndialog_name|editsign\ntext|proxy"
- /send command to send the saved action.
