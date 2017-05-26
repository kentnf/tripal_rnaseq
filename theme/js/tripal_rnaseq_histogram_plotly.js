
//console.log(experiments);
//console.log(rnaseq_exps);

rnaseq_histogram(rnaseq_exps,experiments);

/**
 * plot histogram according to input factors
 */
function rnaseq_histogram_plot(title_name, type_name, factor1, factor2, experiments, values, sd) {

	// plot histogram for groups of factor1 and factor2
	var group = new Object();
	if (typeof factor1 != 'undefined' && typeof factor2 != 'undefined') {
		for (exp_id in experiments) {
			if (typeof experiments[exp_id][factor1] == 'undefined' || typeof experiments[exp_id][factor2] == 'undefined') {
				//console.log("Error, the factor " + factor1 + " or " + factor2 + " is not defined in experiment");
			}
			var name1 = experiments[exp_id][factor1];
			var name2 = experiments[exp_id][factor2];
			if (typeof group[name1] == 'undefined') {
				group[name1] = new Object();
			}
			if (typeof group[name1][name2] == 'undefined') {
				group[name1][name2] = new Array();
			}
			group[name1][name2].push(exp_id);
		}
		console.log(factor1);
		console.log(factor2);
		console.log(group);

		var trace_obj = new Object();
		for (name1 in group) {
			var num = 0;
			for (name2 in group[name1]) {
				for (var i in group[name1][name2]) {
					//var x_name = new Array();
					//var y_value = new Array();
					//var s_dev = new Array();
					if (typeof trace_obj[num] == 'undefined') {
						trace_obj[num] = {
							x: [],
							y: [],
							name: name2,
							type: 'bar',
							error_y: {
								type: 'data',
								array: [],
								visible: true
							},
                    	};
				
					}

					var exp_id = group[name1][name2][i];
					var s = 0;
					if (typeof sd[exp_id] != 'undefined') {
						s = sd[exp_id]; 
					} 
					trace_obj[num].x.push(name1);
					trace_obj[num].y.push(values[exp_id]);
					trace_obj[num].error_y.array.push(s);	
					num++;
				}
			}
		}

		var trace_data = new Array();
		for(var key in trace_obj) {
			trace_data.push(trace_obj[key]);
		}
		//console.log(trace_data);

		var layout = {
			title: title_name,
			yaxis: { title: type_name },
			barmode: 'group'};
        Plotly.newPlot('tripal-rnaseq-histogram', trace_data, layout);
	}
	// plot histogram for each sample
	else {

		x_name = new Array();
		y_value = new Array();
		y_sd = new Array();

		for (var exp_id in experiments) {
			var exp_name = experiments[exp_id].name;
            var value = values[exp_id];
            var sd_value = 0;
            if ( typeof sd[exp_id] != 'undefined') {
                sd_value = sd[exp_id];
            }
            x_name.push(exp_name);
            y_value.push(value);
            y_sd.push(sd_value);
        }
        
		var trace1 = { 
				x: x_name, 
				y: y_value, 
				name: 'RPKM',
				error_y: {
              	  type: 'data',
              	  array: y_sd,
              	  visible: true
            	},
            	type: 'bar'
		};
		var trace_data = [trace1];

		var layout = {
            title: title_name,
            yaxis: { title: type_name },
            barmode: 'group'};
        Plotly.newPlot('tripal-rnaseq-histogram', trace_data, layout);
	
	}
}

/**
 * plot histogram
 */
function rnaseq_histogram(rnaseq_exps, experiments) {

	// order of default value type
	var value_types = new Array('RPKM', 'FPKM', 'RPM', 'raw_count');

	// construct select options for bioproject
	var bioproject_opts = '<option> -select BioProject- </option>\n';
	var type_opts = ''; // not use 

	for (var project_id in rnaseq_exps ) {
		bioproject_opts += "<option value=\""+ project_id + "\">" + rnaseq_exps[project_id]['name'] + "</option>\n";
	}

	bioproject_opts = "<label for=\"bioproject-id\">Select Project &nbsp;&nbsp;</label><select id=\"bioproject-id\" class=\"form-control\">\n" + bioproject_opts + "</select>\n";

	jQuery('#tripal-rnaseq-form-select').html(bioproject_opts);

	jQuery('#bioproject-id').on('change', function() {
        
		var project_id = jQuery('#bioproject-id').val();
		var title_name = rnaseq_exps[project_id].name;
		var designs = rnaseq_exps[project_id].designs;

        // get expression values and SD by the order of value_types 
        var type_name; 
		var sd_name;
        var values; 
		var sd = new Array;
        for (var type in value_types) {
			type_name = value_types[type];
			sd_name = 'SD_' + type_name;
            if (typeof rnaseq_exps[project_id][type_name] !== 'undefined') {
                values = rnaseq_exps[project_id][type_name];
                if (typeof rnaseq_exps[project_id][sd_name] !== 'undefined') {
                    sd = rnaseq_exps[project_id][sd_name];
                }
                break;
            }
        }

		// plot sample expression by default
		var factor1 = undefined;
		var factor2 = undefined;
		rnaseq_histogram_plot(title_name, type_name, factor1, factor2, experiments, values, sd);

		// construct select options for matrix design
		if (typeof rnaseq_exps[project_id].designs != 'undefined') {
			var designs = rnaseq_exps[project_id].designs;
			var matrix_opts = '<option value=\"0\"> -select- </option>\n'; 
			for (var design_num in designs) {
				design = designs[design_num];
				design_var = design[0] + ' vs ' + design[1];
				matrix_opts += '<option value="' + design_num + '">' + design_var + '</option>\n';
			}
			matrix_opts = "<label for=\"biomatrix-id\">Select Factors &nbsp;&nbsp;</label><select id=\"biomatrix-id\" class=\"form-control\">\n" + matrix_opts + "</select>\n";
			jQuery('#tripal-rnaseq-matrix-select').html(matrix_opts);

			// update image when the matrix changed
			jQuery('#biomatrix-id').on('change', function() {

				var matrix_id = jQuery('#biomatrix-id').val();
				var factor1 = undefined;
				var factor2 = undefined;
				if (matrix_id > 0) {
					factor1 = designs[matrix_id][0];
					factor2 = designs[matrix_id][1];
				} 
				rnaseq_histogram_plot(title_name, type_name, factor1, factor2, experiments, values, sd);
			});
		} 
		else {
			jQuery('#tripal-rnaseq-matrix-select').html('');
		}

		console.log(jQuery('#tripal-rnaseq-matrix-select'));
	});
}

