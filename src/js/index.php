<?
$version=0;
if(isset($_GET[version])){
	$version=$_GET[version];
}
?>
<html>
<body>
<script type="text/javascript">
var lang="ru";
if (typeof window.localStorage !== "undefined") {
	if (window.localStorage.pebble_tcw_lang) {
		lang=window.localStorage.pebble_tcw_lang;
	}
}
location.replace("settcw.php?lang="+lang+"&version="+<?echo $version;?>);
</script>
</body>
</html>