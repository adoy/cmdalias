#!/bin/bash

_cmdalias() {
  local COMP_FUNCTION COMP_LWORD

  declare -F _completion_loader &>/dev/null || {
    source /usr/share/bash-completion/bash_completion
  }

  if [[ ${COMP_WORDS[-1]} = '' ]]; then
    COMP_LINE="$(cmdalias -e -- $COMP_LINE) "
    COMP_POINT=${#COMP_LINE}
    read -r -a COMP_WORDS <<< "$COMP_LINE"
    COMP_WORDS+=('')
  else
    COMP_LWORD=${COMP_WORDS[-1]}
    unset COMP_WORDS[${#COMP_WORDS[@]}-1]
    COMP_LINE="$(cmdalias -e -- ${COMP_WORDS[@]}) $COMP_LWORD"
    COMP_POINT=${#COMP_LINE}
    read -r -a COMP_WORDS <<< "$COMP_LINE"
  fi
  COMP_CWORD=$((${#COMP_WORDS[@]}-1))

  COMP_FUNCTION=$(complete -p "${COMP_WORDS[0]}" 2>/dev/null | awk '{print $(NF-1)}')

  if [[ -z $COMP_FUNCTION ]]; then
    _completion_loader "${COMP_WORDS[0]}"
    COMP_FUNCTION=$(complete -p "${COMP_WORDS[0]}" 2>/dev/null | awk '{print $(NF-1)}')
  fi

  if [[ -z $COMP_FUNCTION ]]; then
    return 1
  fi

  $COMP_FUNCTION
}
