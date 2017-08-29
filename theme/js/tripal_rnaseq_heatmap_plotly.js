
rnaseq_heatmap(expression);

function rnaseq_heatmap(expression) {

    console.log(expression);

	z_value = new Array();
	x_gene = new Array();
	y_sample = new Array();
	text = new Array();
	var num = 0;
	var num_gene = 0;
	var num_sample = 0;
	for (gene in expression) {
		num_gene++;
		link = '<a href="/feature/gene/' + gene + '">' + gene + '</a>';
		x_gene.push(link);
		var values = new Array();
		var txts = new Array();
		num++;
		for (sample in expression[gene] ) {
			if (num_gene == 1) {
				num_sample++;
			}
			var value = Math.log2(expression[gene][sample]).toFixed(2);
			txt = 'Sample: ' + sample + '<br>' + 'Gene: ' + gene + '<br>' + 'Value: ' + value;
			values.push(value);
			txts.push(txt);

			if (num == 1) {
				y_sample.push(sample);
			}
		}
		z_value.push(values);
		text.push(txts);
	}

	var data = [
  	  {
		z: z_value,
		x: y_sample,
		y: x_gene,
		type: 'heatmap',
		text: text,
		hoverinfo: 'text',
  	  }
  	];

	var height = 500;
	if (num_gene > 20) {
		height = (num_gene - 20) * 18 + 500;
	}

	var layout = {
        height: height,
		xaxis: {title: 'Samples', side: 'top'},
		yaxis: {title: 'Expression of Genes (Log2RPKM)'},
		margin: {l: 180, b: 20},
		legend: {},
	};

	Plotly.newPlot('tripal-rnaseq-heatmap', data, layout);
}

