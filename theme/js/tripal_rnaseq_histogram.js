
jQuery(function(){



	
	var img_height = 380;
	var default_bar_width = 20;
	var margin = 65;

	var options = ["1", "2", "3", "4", "5"];
	jQuery('#select-project').empty();
	jQuery.each(options, function(i, p) {
    	jQuery('#select-project').append(jQuery('<option></option>').val(p).html(p));
	});
        
	//$('#submit').click( function(){
		//var tid = $('#transID').val();
		//var histData = load_data(tid);
        var histData = { 
			'data1' : {
            	"express":[ "10", "20", "30" ],
            	"sample":[ "John", "Anna", "Peter" ],
			},
			'data2' : {
				"express":[ "10", "20", "30" ],
				"sample":[ "Jo", "An", "Pe" ],
			}
        }
        console.log(rnaseq_exps); 
		// remove prev svg
		d3.select("tripal-rnaseq-histogram").remove();
            
		// plot expression dataset for selected projects
		var data_i = 0;
		for (var project_id in rnaseq_exps) {

			data_i++;
			var sample  = [];
			var express = [];
       
			for(var experiment_id in rnaseq_exps[project_id]['RPM']) {
				name = experiments[experiment_id]['name'];
				exp_value = rnaseq_exps[project_id]['RPM'][experiment_id];
				sample.push(name);
				express.push(exp_value);
			}

			// set svg size
			var img_height = 380;
			var bar_width = 20;
			var bar_padding = 1;
			var margin_top = 20;
			var margin_right = 20;
			var margin_left = 65;
			var margin_bottom = 65;
                
			var img_width = sample.length * (bar_width + bar_padding);
			var svg = d3.select("#tripal-rnaseq-histogram")
				.append("svg")
				.attr("width", img_width + margin_right + margin_left)
				.attr("height", img_height + margin_top + margin_bottom);

			//setting scale
			var max = d3.max(express, function(d) { return d; });
			var xscale = d3.scale.linear().domain([0, sample.length]).range([0, img_width]);
			var yscale = d3.scale.linear().domain([max, 0]).range([0, img_height]);

			// setting axis
			var xaxis = d3.svg.axis().scale(xscale).ticks(sample.length).tickFormat(function(d, i){ return sample[i]; }).orient("bottom");  //for sample
			var yaxis = d3.svg.axis().scale(yscale).ticks(5).orient("left");        // for expression

			// draw bar
			console.log(express);
			svg.selectAll("rect")
				.data(express)
				.enter()
				.append("rect")
				.attr("x", function(d, i) { return xscale(i) + margin_left; })
				.attr("y", function(d) {return margin_top + yscale(d); } )
				.attr("width", (default_bar_width - 1))
				.attr("height", function(d) { return img_height - yscale(d); })
				.attr("fill", "teal");

			var x_pos = margin_top + img_height;
			svg.append("g")
				.attr("class", "x axis")
				.attr("transform", "translate(" + margin_left + ", " + x_pos + ")")
				//.style({'fill':'none'})
				.call(xaxis)
				.selectAll("text")
					.style("text-anchor", "end")
					.attr("dx", "-.8em")
					.attr("dy", ".15em")
					.attr("transform", function(d) {
						return "rotate(-90)" 
					});

			svg.append("g")
				.attr("class", "y axis")
				.attr("transform", "translate(" + margin_left + ", " + margin_top +" )")
				.call(yaxis);
                
			//break;
		}
	//});
});

