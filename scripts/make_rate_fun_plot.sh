#!/bin/bash

if [ $# -gt 2 ] || [ $# -lt 1 ]
then
    echo "usage $0: PLOT_FILES_PREFIX [MINIMAL_FOCUS_STEP]"
    exit 1
fi

AUTOFOCUS_BINARY_FILE=../build/autofocus

THIS_EXPOSITION_NAME=""
PLOT_DATA_PREFIX=$1
MIN_FOCUS_STEP=$2
PLOT_DATA_POSTFIX=".data"
PLOT_VIDEO_POSTFIX=".avi"
PLOT_FILENAME=${PLOT_DATA_PREFIX}.gnuplot

if [[ -z ${MIN_FOCUS_STEP} ]]
then
    MIN_FOCUS_STEP=10
fi

cat << EOF > ${PLOT_FILENAME}
    set title "Evaluation of exposition shape rating function"
    set xlabel "Samples"
    set ylabel "Rating"

EOF
echo -n -e "    plot " >> ${PLOT_FILENAME}


while read -p "Next test name (hit ENTER to quit): " THIS_EXPOSITION_NAME && [[ ! -z ${THIS_EXPOSITION_NAME} ]]
do
    CURRENT_DATA_FILE=${PLOT_DATA_PREFIX}${THIS_EXPOSITION_NAME}${PLOT_DATA_POSTFIX}
    CURRENT_VIDEO_FILE=${PLOT_DATA_PREFIX}${THIS_EXPOSITION_NAME}${PLOT_VIDEO_POSTFIX}
    echo "Capturing ${THIS_EXPOSITION_NAME}, into ${CURRENT_DATA_FILE}, with video in ${CURRENT_VIDEO_FILE}."
    ${AUTOFOCUS_BINARY_FILE} -m -d ${MIN_FOCUS_STEP} -o ${CURRENT_VIDEO_FILE} 1> ${CURRENT_DATA_FILE} 2>/dev/null
    echo -n -e "'${CURRENT_DATA_FILE}' with lines title '${THIS_EXPOSITION_NAME}', " >> ${PLOT_FILENAME}
done

cat << EOF >> $PLOT_FILENAME

    pause -1
EOF
