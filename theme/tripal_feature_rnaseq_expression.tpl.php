<?php

$feature = $variables['node']->feature;

if ($feature->type_id->name == 'gene') {
	?>
	<div class="row">
		<div class="col-md-8">

		<form class="form-inline">
      		<div id="tripal-rnaseq-form-select" class="form-group">
      		</div>
      		&nbsp;&nbsp;&nbsp;
      		<div id="tripal-rnaseq-matrix-select" class="form-group">
      		</div>
    	</form>
    
		<br>
    	<div id=tripal-rnaseq-histogram></div>
		<br>
    	<div id=tripal-rnaseq-description></div>
		<br>
		</div>
	</div>
<?php
}

