<?php
require("server_config.php");
header('Content-type: application/x-java-jnlp-file');
echo("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
?>
<jnlp spec="1.0+" codebase="http://<?php echo($server) ?>/">
<information>
<title>The OPENPKMN Client</title>
<vendor>The OPENPKMN Project</vendor>
<homepage href="http://pkmn.org" />
<description kind="short">Pokemon RBY Battle Simulator</description>
<description>The client for The OPENPKMN Project's open source Pokemon Red/Blue/Yellow Battle Simulator</description>
</information>
<resources>
<j2se version="1.6+" />
<jar href="OpenpkmnClient.jar" />
</resources>
<security>
<all-permissions />
</security>
<application-desc main-class="openpkmnclient.Main">
<argument><?php echo($_COOKIE["username"])?></argument>
<argument><?php echo($_COOKIE["password"])?></argument>
<argument><?php echo($server)?></argument>
<argument><?php echo($port)?></argument>
</application-desc>
</jnlp>
