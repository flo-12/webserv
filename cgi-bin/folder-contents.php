<?php
$folderPath = __DIR__ . '/uploads';

$files = scandir($folderPath);

// Remove '.' and '..' from the list
$files = array_diff($files, array('.', '..'));

// Convert to a simple array and encode to JSON
echo json_encode(array_values($files));
?>