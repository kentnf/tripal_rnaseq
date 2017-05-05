
console.log(experiments);
console.log(rnaseq_exps);


rnaseq_histogram(rnaseq_exps,experiments);

function rnaseq_histogram(rnaseq_exps, experiments) {

   /*
       <select id="select-project">
          <option>Choose a BioProject</option>
        </select>

        <select id="select-type">
          <option>Choose a Expression Type</option>
        </select>
      
        <select id="select-matrix">
          <option>Choose a Matrix</option>
        </select>
    */

	// order of default value type
	var value_types = new Array('RPKM', 'FPKM', 'RPM', 'raw_count');


	// construct select form
	var bioproject_opts = '<option>-select BioProject-</option>\n';
	var type_opts = '';
	var matrix_opts = '<option>-select -</option>\n'; 

	for (var project_id in rnaseq_exps ) {
		bioproject_opts += "<option value=\""+ project_id + "\">" + rnaseq_exps[project_id]['name'] + "</option>\n";
	}

	bioproject_opts = "<select id=\"bioproject-id\">\n" + bioproject_opts + "</select>\n";

	jQuery('#tripal-rnaseq-form-select').html(bioproject_opts);

	jQuery('#bioproject-id').on('change', function(){
		var project_id = jQuery('#bioproject-id').val();

		// update histogram according to bioproject_id
		// set default value by the order of value_types 
		var type_name; var sd_name;
		var values; var sd = new Array;
		for (var type in value_types) {
			if (typeof rnaseq_exps[project_id][value_types[type]] !== 'undefined') {
				type_name = value_types[type];
				sd_name = 'SD_' + type_name;
				values = rnaseq_exps[project_id][type_name];
				if (typeof rnaseq_exps[project_id][sd_name] !== 'undefined') {
					sd = rnaseq_exps[project_id][sd_name];
				}
				break;
			}
		}

		var x_name  = new Array;
		var y_value = new Array;
		var y_sd    = new Array;

		for (var experiment_id in values) {
			var value = values[experiment_id];
			var name = experiments[experiment_id]['name'];	
			var sd_value = 0;
			if (sd[experiment_id] != 'undefined') {
				sd_value = sd[experiment_id] * 0.1;
			}
			x_name.push(name);
			y_value.push(value);
			y_sd.push(sd_value);
		}
		var trace1 = { x: x_name, y: y_value, name: type_name,
			error_y: {
    		  type: 'data',
    		  array: y_sd,
    		  visible: true
  			},
			type: 'bar'
		};

		var data = [trace1];
		var layout = {
			title: rnaseq_exps[project_id]['name'],
			yaxis: { title: type_name },
			barmode: 'group'};
    	Plotly.newPlot('tripal-rnaseq-histogram', data, layout);
	});

}


 
