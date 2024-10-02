@MODULE:lwDashCore /* Підключаємо основні модулі lwDash. */
@MODULE:uk_skull_bg

@SECTION:body.html
<div class="hint dialog" id="version-hint" data-template="Тут зазначена версія: @ENV:GIT_REPO_VERSION"></div>
<div id="content"></div>

@SECTION:script.js
//FIRMWARE UPDATE
let firmPane = lwDashPane(_("content"));
lwDashTemplate(firmPane.title, "UPDATE");
firmPane.content.style.flexDirection = "column";
firmPane.content.style.flex = "100%";
firmPane.content.innerHTML = `
	<iframe name= "hidden-iframe" style= "display: none;"></iframe>
	<form method= "post" enctype= "multipart/form-data" target = "hidden-iframe" action = "update" style= "display:contents">
		<input type= "file" class = "layout" id= "file" name= "file">
		<button class = "button">Підтвердити</button>
	</form>
`
