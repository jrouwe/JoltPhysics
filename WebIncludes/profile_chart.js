var canvas;
var ctx;
var tooltip;
var min_scale;
var scale;
var offset_x = 0;
var offset_y = 0;
var size_y;
var dragging = false;
var previous_x = 0;
var previous_y = 0;
var bar_height = 15;
var line_height = bar_height + 2;
var thread_separation = 6;
var thread_font_size = 12;
var thread_font = thread_font_size + "px Arial";
var bar_font_size = 10;
var bar_font = bar_font_size + "px Arial";
var end_cycle = 0;

function drawChart()
{
	ctx.clearRect(0, 0, canvas.width, canvas.height);

	ctx.lineWidth = 1;
	
	var y = offset_y;
	
	for (var t = 0; t < threads.length; t++)		
	{
		// Check if thread has samples
		var thread = threads[t];
		if (thread.start.length == 0)
			continue;
		
		// Draw thread name
		y += thread_font_size;
		ctx.font = thread_font;
		ctx.fillStyle = "#000000";
		ctx.fillText(thread.thread_name, 0, y);
		y += thread_separation;

		// Draw outlines for each bar of samples
		ctx.fillStyle = "#c0c0c0";
		for (var d = 0; d <= thread.max_depth; d++)
			ctx.fillRect(0, y + d * line_height, canvas.width, bar_height);
		
		// Draw samples
		ctx.font = bar_font;
		for (var s = 0; s < thread.start.length; s++)
		{
			// Cull bar
			var rx = scale * (offset_x + thread.start[s]);
			if (rx > canvas.width) // right of canvas
				break;			
			var rw = scale * thread.cycles[s];	
			if (rw < 0.5) // less than half pixel, skip
				continue;
			if (rx + rw < 0) // left of canvas
				continue;
			
			// Draw bar
			var ry = y + line_height * thread.depth[s];
			ctx.fillStyle = thread.color[s];		
			ctx.fillRect(rx, ry, rw, bar_height);
			ctx.strokeStyle = thread.darkened_color[s];
			ctx.strokeRect(rx, ry, rw, bar_height);
			
			// Get index in aggregated list
			var a = thread.aggregator[s];

			// Draw text
			if (rw > aggregated.name_width[a])
			{
				ctx.fillStyle = "#000000";
				ctx.fillText(aggregated.name[a], rx + (rw - aggregated.name_width[a]) / 2, ry + bar_height - 4);
			}
		}

		// Next line
		y += line_height * (1 + thread.max_depth) + thread_separation;
	}
	
	// Update size
	size_y = y - offset_y;
}

function drawTooltip(mouse_x, mouse_y)
{
	var y = offset_y;
	
	for (var t = 0; t < threads.length; t++)		
	{
		// Check if thread has samples
		var thread = threads[t];
		if (thread.start.length == 0)
			continue;
		
		// Thead name
		y += thread_font_size + thread_separation;

		// Draw samples
		for (var s = 0; s < thread.start.length; s++)
		{
			// Cull bar
			var rx = scale * (offset_x + thread.start[s]);
			if (rx > mouse_x)
				break;			
			var rw = scale * thread.cycles[s];
			if (rx + rw < mouse_x)
				continue;
			
			var ry = y + line_height * thread.depth[s];			
			if (mouse_y >= ry && mouse_y < ry + bar_height)
			{
				// Get index into aggregated list
				var a = thread.aggregator[s];
				
				// Found bar, fill in tooltip
				tooltip.style.left = (canvas.offsetLeft + mouse_x) + "px";
				tooltip.style.top = (canvas.offsetTop + mouse_y) + "px";
				tooltip.style.visibility = "visible";
				tooltip.innerHTML = aggregated.name[a] + "<br>"
					+ "<table>"
					+ "<tr><td>Time:</td><td class=\"stat\">" + (1000000 * thread.cycles[s] / cycles_per_second).toFixed(2) + " &micro;s</td></tr>"
					+ "<tr><td>Start:</td><td class=\"stat\">" + (1000000 * thread.start[s] / cycles_per_second).toFixed(2) + " &micro;s</td></tr>"
					+ "<tr><td>End:</td><td class=\"stat\">" + (1000000 * (thread.start[s] + thread.cycles[s]) / cycles_per_second).toFixed(2) + " &micro;s</td></tr>"
					+ "<tr><td>Avg. Time:</td><td class=\"stat\">" + (1000000 * aggregated.cycles_per_frame[a] / cycles_per_second / aggregated.calls[a]).toFixed(2) + " &micro;s</td></tr>"
					+ "<tr><td>Min Time:</td><td class=\"stat\">" + (1000000 * aggregated.min_cycles[a] / cycles_per_second).toFixed(2) + " &micro;s</td></tr>"
					+ "<tr><td>Max Time:</td><td class=\"stat\">" + (1000000 * aggregated.max_cycles[a] / cycles_per_second).toFixed(2) + " &micro;s</td></tr>"
					+ "<tr><td>Time / Frame:</td><td class=\"stat\">" + (1000000 * aggregated.cycles_per_frame[a] / cycles_per_second).toFixed(2) + " &micro;s</td></tr>"
					+ "<tr><td>Calls:</td><td class=\"stat\">" + aggregated.calls[a] + "</td></tr>"
					+ "</table>";
				return;
			}			
		}

		// Next line
		y += line_height * (1 + thread.max_depth) + thread_separation;
	}
	
	// No bar found, hide tooltip
	tooltip.style.visibility = "hidden";
}

function onMouseDown(evt) 
{
	dragging = true;
	previous_x = evt.clientX, previous_y = evt.clientY;
	tooltip.style.visibility = "hidden";
}

function onMouseUp(evt) 
{
	dragging = false;
}

function clampMotion()
{
	// Clamp horizontally
	var min_offset_x = canvas.width / scale - end_cycle;
	if (offset_x < min_offset_x)
		offset_x = min_offset_x;
	if (offset_x > 0)
		offset_x = 0;
	
	// Clamp vertically
	var min_offset_y = canvas.height - size_y;
	if (offset_y < min_offset_y)
		offset_y = min_offset_y;
	if (offset_y > 0)
		offset_y = 0;

	// Clamp scale
	if (scale < min_scale)
		scale = min_scale;
	var max_scale = 1000 * min_scale;
	if (scale > max_scale)
		scale = max_scale;
}

function onMouseMove(evt) 
{
	if (dragging)
	{
		// Calculate new offset
		offset_x += (evt.clientX - previous_x) / scale;
		offset_y += evt.clientY - previous_y;

		clampMotion();
		
		drawChart();
	}
	else
		drawTooltip(evt.clientX - canvas.offsetLeft, evt.clientY - canvas.offsetTop);

	previous_x = evt.clientX, previous_y = evt.clientY;
}

function onScroll(evt)
{
	tooltip.style.visibility = "hidden";

	var old_scale = scale;
	if (evt.deltaY > 0)
		scale /= 1.1;
	else
		scale *= 1.1;
	
	clampMotion();
	
	// Ensure that event under mouse stays under mouse
	var x = previous_x - canvas.offsetLeft;	
	offset_x += x / scale - x / old_scale;

	clampMotion();

	drawChart();
}

function darkenColor(color) 
{   
    var i = parseInt(color.slice(1), 16);
	
	var r = i >> 16;
	var g = (i >> 8) & 0xff;
	var b = i & 0xff;
	
	r = Math.round(0.8 * r);
	g = Math.round(0.8 * g);
	b = Math.round(0.8 * b);
	
	i = (r << 16) + (g << 8) + b;
	
	return "#" + i.toString(16);
}

function startChart()
{
	// Fetch elements
	canvas = document.getElementById('canvas');
	ctx = canvas.getContext("2d");
	tooltip = document.getElementById('tooltip');
	
	// Resize canvas to fill screen
	canvas.width = document.body.offsetWidth - 20;
	canvas.height = document.body.offsetHeight - 20;
	
	// Register mouse handlers
	canvas.onmousedown = onMouseDown;
	canvas.onmouseup = onMouseUp;
	canvas.onmouseout = onMouseUp;
	canvas.onmousemove = onMouseMove;
	canvas.onwheel  = onScroll;
	
	for (var t = 0; t < threads.length; t++)		
	{
		var thread = threads[t];

		// Calculate darkened colors
		thread.darkened_color = new Array(thread.color.length);
		for (var s = 0; s < thread.color.length; s++)
			thread.darkened_color[s] = darkenColor(thread.color[s]);
		
		// Calculate max depth and end cycle
		thread.max_depth = 0;
		for (var s = 0; s < thread.start.length; s++)
		{
			thread.max_depth = Math.max(thread.max_depth, thread.depth[s]);
			end_cycle = Math.max(end_cycle, thread.start[s] + thread.cycles[s]);
		}
	}
	
	// Calculate width of name strings
	ctx.font = bar_font;
	aggregated.name_width = new Array(aggregated.name.length);
	for (var a = 0; a < aggregated.name.length; a++)
		aggregated.name_width[a] = ctx.measureText(aggregated.name[a]).width;

	// Store scale properties
	min_scale = canvas.width / end_cycle;
	scale = min_scale;
	
	drawChart();
}