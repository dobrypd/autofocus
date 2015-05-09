set terminal png size 1920,1080 enhanced font 'Verdana' 10
set output 'NikonD90-50mm1.4G-evaluation.png'

set title "Evaluation of exposition shape rating function"
    set xlabel "Samples"
    set ylabel "Rating"

    plot 'NikonD90-50mm1.4G-plant.data' with lines title 'plant', 'NikonD90-50mm1.4G-window.data' with lines title 'window', 'NikonD90-50mm1.4G-wall.data' with lines title 'wall', 'NikonD90-50mm1.4G-texture.data' with lines title 'texture', 'NikonD90-50mm1.4G-tee.data' with lines title 'tee', 'NikonD90-50mm1.4G-texture_too_close.data' with lines title 'texture_too_close'
