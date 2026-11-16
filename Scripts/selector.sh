#!/bin/sh
#Code adapted from NPTOOL:
#https://gitlab.in2p3.fr/np/nptool/-/blob/NPTool.2.dev/nptool.sh

NPARCH="$(uname -s)"

# test if export is supported
export 1>/dev/null 2>/dev/null
if [ $? -eq 0 ]; then
  CMD="export"
  SEP="="
else
  setenv 1>/dev/null 2>/dev/null
  if [ "${?} == 0" ]; then
  CMD="setenv"
  SEP=" "
  else
  echo "Neither setenv nor export found!"
  fi
fi 

# find script path
if [ -n "$ZSH_VERSION" ]; then
   SCRIPTPATH="$( cd "$( dirname "${(%):-%x}" )/.." && pwd )"
elif [ -n "$tcsh" ]; then
   SCRIPTPATH="$( cd "$( dirname "$0" )/.." && pwd )"
elif [ -n "$BASH_VERSION" ]; then
   SCRIPTPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
else
   echo "neither bash or zsh is used, abort"
   exit 1
fi

# export AGATASELECTOR environment variable
${CMD} AGATASELECTOR${SEP}$SCRIPTPATH
if [ "${NPARCH}" = "Darwin" ] ;
then
  ${CMD} DYLD_LIBRARY_PATH${SEP}$AGATASELECTOR/src/Container:$DYLD_LIBRARY_PATH
  ${CMD} DYLD_LIBRARY_PATH${SEP}$AGATASELECTOR/src/Optimizer:$DYLD_LIBRARY_PATH
else 
  ${CMD} LD_LIBRARY_PATH${SEP}$AGATASELECTOR/src/Container:$LD_LIBRARY_PATH
  ${CMD} LD_LIBRARY_PATH${SEP}$AGATASELECTOR/src/Optimizer:$LD_LIBRARY_PATH
fi

${CMD} PATH=$AGATASELECTOR:$PATH

SCRIPTS_SUBPATH="/Scripts/AncMerging"
${CMD} PATH=$AGATASELECTOR${SCRIPTS_SUBPATH}:$PATH


# Tab completion for options
OPTIONS="
--help 
-h 
--nrevts
--nrthr
--conf
--iofile
--iolistfile
--verb
--no_user_sel
--print_conf
--rm_partial
--optimize
--only_enabled_histos
--set_gates
--debug_canvas
--mem_check
--reduction_cond
--distribute
--no_hadd
--update_runs
--sum_all
--subtract_bkg
--update_prisma
--window_size
--spy
"

_special_completion() {
  local cur prev
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD-1]}"

  # File-specific completions
  case "${prev}" in
    --conf|--print_conf|--iolistfile|--iofile|--set_gates)
      COMPREPLY=( $(compgen -f -- "${cur}") )
      ;;
    *)
      # Default: Complete with the list of options
      COMPREPLY=( $(compgen -W "${OPTIONS}" -- "${cur}") )
      ;;
  esac
}

# Register the completion function
if [[ "$(uname)" == "Darwin" ]]; then
  if ! type complete &> /dev/null; then
    echo ""
  else
    complete -F _special_completion RunSelector
  fi
else
  complete -F _special_completion RunSelector
fi
