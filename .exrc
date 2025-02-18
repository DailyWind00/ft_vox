if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <Plug>(-fzf-complete-finish) l
inoremap <silent> <Plug>(fzf-maps-i) :call fzf#vim#maps('i', 0)
inoremap <expr> <Plug>(fzf-complete-buffer-line) fzf#vim#complete#buffer_line()
inoremap <expr> <Plug>(fzf-complete-line) fzf#vim#complete#line()
inoremap <expr> <Plug>(fzf-complete-file-ag) fzf#vim#complete#path('ag -l -g ""')
inoremap <expr> <Plug>(fzf-complete-file) fzf#vim#complete#path("find . -path '*/\.*' -prune -o -type f -print -o -type l -print | sed 's:^..::'")
inoremap <expr> <Plug>(fzf-complete-path) fzf#vim#complete#path("find . -path '*/\.*' -prune -o -print | sed '1d;s:^..::'")
inoremap <expr> <Plug>(fzf-complete-word) fzf#vim#complete#word()
nnoremap <silent>  :LspDeclaration
nnoremap <silent>  :Files
nnoremap <silent>  :LspDefinition
xmap gx <Plug>NetrwBrowseXVis
nmap gx <Plug>NetrwBrowseX
nnoremap <Plug>(-fzf-vim-do) :execute g:__fzf_command
nnoremap <Plug>(-fzf-/) /
nnoremap <Plug>(-fzf-:) :
nnoremap <silent> <Plug>(-fzf-complete-finish) a
nmap <F1> :Stdheader
xnoremap <silent> <Plug>NetrwBrowseXVis :call netrw#BrowseXVis()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#BrowseX(netrw#GX(),netrw#CheckIfRemote(netrw#GX()))
tnoremap <silent> <Plug>(fzf-normal) 
tnoremap <silent> <Plug>(fzf-insert) i
nnoremap <silent> <Plug>(fzf-normal) <Nop>
nnoremap <silent> <Plug>(fzf-insert) i
nnoremap <silent> <Plug>(lsp-signature-help) :call lsp#ui#vim#signature_help#get_signature_help_under_cursor()
nnoremap <silent> <Plug>(lsp-previous-reference) :call lsp#internal#document_highlight#jump(-1)
nnoremap <silent> <Plug>(lsp-next-reference) :call lsp#internal#document_highlight#jump(+1)
nnoremap <silent> <Plug>(lsp-status) :echo lsp#get_server_status()
nnoremap <silent> <Plug>(lsp-peek-implementation) :call lsp#ui#vim#implementation(1)
nnoremap <silent> <Plug>(lsp-implementation) :call lsp#ui#vim#implementation(0)
xnoremap <silent> <Plug>(lsp-document-range-format) :<Home>silent <End>call lsp#internal#document_range_formatting#format({ 'bufnr': bufnr('%') })
nnoremap <silent> <Plug>(lsp-document-range-format) :set opfunc=lsp#internal#document_range_formatting#opfuncg@
vnoremap <silent> <Plug>(lsp-document-format) :<Home>silent <End>call lsp#internal#document_range_formatting#format({ 'bufnr': bufnr('%') })
nnoremap <silent> <Plug>(lsp-document-format) :call lsp#internal#document_formatting#format({ 'bufnr': bufnr('%') })
nnoremap <silent> <Plug>(lsp-workspace-symbol-search) :call lsp#internal#workspace_symbol#search#do({})
nnoremap <silent> <Plug>(lsp-workspace-symbol) :call lsp#ui#vim#workspace_symbol('')
nnoremap <silent> <Plug>(lsp-peek-type-definition) :call lsp#ui#vim#type_definition(1)
nnoremap <silent> <Plug>(lsp-type-hierarchy) :call lsp#internal#type_hierarchy#show()
nnoremap <silent> <Plug>(lsp-type-definition) :call lsp#ui#vim#type_definition(0)
nnoremap <silent> <Plug>(lsp-rename) :call lsp#ui#vim#rename()
nnoremap <silent> <Plug>(lsp-references) :call lsp#ui#vim#references({})
nnoremap <silent> <Plug>(lsp-previous-diagnostic-nowrap) :call lsp#internal#diagnostics#movement#_previous_diagnostics("-wrap=0")
nnoremap <silent> <Plug>(lsp-previous-diagnostic) :call lsp#internal#diagnostics#movement#_previous_diagnostics()
nnoremap <silent> <Plug>(lsp-next-diagnostic-nowrap) :call lsp#internal#diagnostics#movement#_next_diagnostics("-wrap=0")
nnoremap <silent> <Plug>(lsp-next-diagnostic) :call lsp#internal#diagnostics#movement#_next_diagnostics()
nnoremap <silent> <Plug>(lsp-previous-warning-nowrap) :call lsp#internal#diagnostics#movement#_previous_warning("-wrap=0")
nnoremap <silent> <Plug>(lsp-previous-warning) :call lsp#internal#diagnostics#movement#_previous_warning()
nnoremap <silent> <Plug>(lsp-next-warning-nowrap) :call lsp#internal#diagnostics#movement#_next_warning("-wrap=0")
nnoremap <silent> <Plug>(lsp-next-warning) :call lsp#internal#diagnostics#movement#_next_warning()
nnoremap <silent> <Plug>(lsp-previous-error-nowrap) :call lsp#internal#diagnostics#movement#_previous_error("-wrap=0")
nnoremap <silent> <Plug>(lsp-previous-error) :call lsp#internal#diagnostics#movement#_previous_error()
nnoremap <silent> <Plug>(lsp-next-error-nowrap) :call lsp#internal#diagnostics#movement#_next_error("-wrap=0")
nnoremap <silent> <Plug>(lsp-next-error) :call lsp#internal#diagnostics#movement#_next_error()
nnoremap <silent> <Plug>(lsp-preview-focus) :call lsp#ui#vim#output#focuspreview()
nnoremap <silent> <Plug>(lsp-preview-close) :call lsp#ui#vim#output#closepreview()
nnoremap <silent> <Plug>(lsp-hover-preview) :call lsp#internal#document_hover#under_cursor#do({ 'ui': 'preview' })
nnoremap <silent> <Plug>(lsp-hover-float) :call lsp#internal#document_hover#under_cursor#do({ 'ui': 'float' })
nnoremap <silent> <Plug>(lsp-hover) :call lsp#internal#document_hover#under_cursor#do({})
nnoremap <silent> <Plug>(lsp-document-diagnostics) :call lsp#internal#diagnostics#document_diagnostics_command#do({})
nnoremap <silent> <Plug>(lsp-document-symbol-search) :call lsp#internal#document_symbol#search#do({})
nnoremap <silent> <Plug>(lsp-document-symbol) :call lsp#ui#vim#document_symbol()
nnoremap <silent> <Plug>(lsp-peek-definition) :call lsp#ui#vim#definition(1)
nnoremap <silent> <Plug>(lsp-definition) :call lsp#ui#vim#definition(0)
nnoremap <silent> <Plug>(lsp-peek-declaration) :call lsp#ui#vim#declaration(1)
nnoremap <silent> <Plug>(lsp-declaration) :call lsp#ui#vim#declaration(0)
nnoremap <silent> <Plug>(lsp-code-lens) :call lsp#ui#vim#code_lens()
nnoremap <silent> <Plug>(lsp-code-action-preview) :call lsp#ui#vim#code_action({ 'ui': 'preview' })
nnoremap <silent> <Plug>(lsp-code-action-float) :call lsp#ui#vim#code_action({ 'ui': 'float' })
nnoremap <silent> <Plug>(lsp-code-action) :call lsp#ui#vim#code_action({})
nnoremap <silent> <Plug>(lsp-call-hierarchy-outgoing) :call lsp#ui#vim#call_hierarchy_outgoing()
nnoremap <silent> <Plug>(lsp-call-hierarchy-incoming) :call lsp#ui#vim#call_hierarchy_incoming({})
onoremap <silent> <Plug>(fzf-maps-o) :call fzf#vim#maps('o', 0)
xnoremap <silent> <Plug>(fzf-maps-x) :call fzf#vim#maps('x', 0)
nnoremap <silent> <Plug>(fzf-maps-n) :call fzf#vim#maps('n', 0)
noremap <C-.> :norm _df-
noremap <C-/> :norm I//-
nnoremap <silent> <C-D> :LspDeclaration
nnoremap <silent> <C-S> :LspDefinition
nnoremap <silent> <C-F> :Files
nnoremap <silent> <C-`> :qall!
nnoremap <silent> <F2> :mksession! %:p:h/.session.vim
let &cpo=s:cpo_save
unlet s:cpo_save
set background=dark
set backupdir=~/.cache/vim/backup//
set directory=~/.cache/vim/swap//
set fileencodings=ucs-bom,utf-8,default,latin1
set helplang=en
set laststatus=2
set ruler
set runtimepath=~/.vim,~/.vim/pack/themes/start/dracula,~/.vim/plugged/vim-42header,~/.vim/plugged/falcon,~/.vim/plugged/vim-tomorrow-theme,~/.vim/plugged/Tomorrow-Night-Bright,~/.vim/plugged/vim-dogrun,~/.vim/plugged/fzf.vim,~/.vim/plugged/vim-airline,~/.vim/plugged/vim-lsp,~/.vim/plugged/vim-lsp-settings,/usr/share/vim/vimfiles,/usr/share/vim/vim91,/usr/share/vim/vimfiles/after,~/.vim/plugged/vim-42header/after,~/.vim/after
set splitright
set suffixes=.bak,~,.o,.info,.swp,.aux,.bbl,.blg,.brf,.cb,.dvi,.idx,.ilg,.ind,.inx,.jpg,.log,.out,.png,.toc
set termguicolors
set undodir=~/.cache/vim/undo//
set window=48
" vim: set ft=vim :
