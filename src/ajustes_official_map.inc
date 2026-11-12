/* J6: verified official owners only (docs/settings-restoration/stage-j6-audit.json).
 * hide_unreleased_content is NOT here: Stage G applies it from home_catalog_shared. */
static const char *const aj6Choice30[]={"IN_SEARCH","IN_SIDEBAR","OFF",NULL};
static const char *const aj6Choice31[]={"SHOW_ALL","HIDE_ALL",NULL};
static const char *const aj6Choice32[]={"CARD","WIDE","POSTER",NULL};
static const char *const aj6Choice33[]={"DEFAULT","STREAMING_STYLE","SPLIT_UPCOMING",NULL};
static const char *const aj6Choice34[]={"ALL_SOURCES","INSTALLED_ADDONS_ONLY","ENABLED_PLUGINS_ONLY",NULL};
static const char *const aj6Choice35[]={"WHITE","CRIMSON","OCEAN","VIOLET","EMERALD","AMBER","ROSE","GOLD","JADE","ROSE_GOLD","ARCTIC_BLUE","GRAPHITE",NULL};
static const char *const aj6Choice36[]={NULL,"NUVIO_SYNC","TRAKT","SIMKL",NULL};
static const char *const aj6Choice37[]={"","torbox","premiumize",NULL};
static const Aj6Map aj6Map[]={
 {AJ_NOME_ADDON,"layout_settings","catalog_addon_name_enabled",AJ6_BOOL,NULL,0},
 {AJ_GRAD_CLASSICO,"layout_settings","classic_focus_gradient_enabled",AJ6_BOOL,NULL,0},
 {AJ_DET_TRAILER,"layout_settings","detail_page_trailer_button_enabled",AJ6_BOOL,NULL,0},
 {AJ_PROF,"layout_settings","card_depth_enabled",AJ6_BOOL,NULL,0},
 {AJ_PROF_POSTERS,"layout_settings","card_depth_posters_enabled",AJ6_BOOL,NULL,0},
 {AJ_PROF_CW,"layout_settings","card_depth_continue_watching_enabled",AJ6_BOOL,NULL,0},
 {AJ_PROF_EPS,"layout_settings","card_depth_episode_cards_enabled",AJ6_BOOL,NULL,0},
 {AJ_PROF_ELENCO,"layout_settings","card_depth_cast_enabled",AJ6_BOOL,NULL,0},
 {AJ_PROF_TRAILERS,"layout_settings","card_depth_trailers_enabled",AJ6_BOOL,NULL,0},
 {AJ_MDB_LIGADO,"mdblist_settings","mdblist_enabled",AJ6_BOOL,NULL,0},
 {AJ_MDB_TRAKT,"mdblist_settings","mdblist_show_trakt",AJ6_BOOL,NULL,0},
 {AJ_MDB_IMDB,"mdblist_settings","mdblist_show_imdb",AJ6_BOOL,NULL,0},
 {AJ_MDB_TMDB,"mdblist_settings","mdblist_show_tmdb",AJ6_BOOL,NULL,0},
 {AJ_MDB_LETTER,"mdblist_settings","mdblist_show_letterboxd",AJ6_BOOL,NULL,0},
 {AJ_MDB_TOMATES,"mdblist_settings","mdblist_show_tomatoes",AJ6_BOOL,NULL,0},
 {AJ_MDB_AUDIENCIA,"mdblist_settings","mdblist_show_audience",AJ6_BOOL,NULL,0},
 {AJ_MDB_META,"mdblist_settings","mdblist_show_metacritic",AJ6_BOOL,NULL,0},
 {AJ_MDB_MAL,"mdblist_settings","mdblist_show_mal",AJ6_BOOL,NULL,0},
 {AJ_PLR_CLASSIF,"player_settings","parental_guide_enabled",AJ6_BOOL,NULL,0},
 {AJ_PROXIMO_AUTO,"player_settings","stream_auto_play_next_episode_enabled",AJ6_BOOL,NULL,0},
 {AJ_LINK_REUSAR,"player_settings","stream_reuse_last_link_enabled",AJ6_BOOL,NULL,0},
 {AJ_EXPANDIR_ATRASO,"layout_settings","focused_poster_backdrop_expand_delay_seconds",AJ6_INT,NULL,0},
 {AJ_PROF_BORDA,"layout_settings","card_depth_edge_strength",AJ6_INT,NULL,0},
 {AJ_PROF_BRILHO,"layout_settings","card_depth_sheen_strength",AJ6_INT,NULL,0},
 {AJ_PROF_COBERTURA,"layout_settings","card_depth_edge_coverage",AJ6_INT,NULL,0},
 {AJ_LARGURA_DP,"layout_settings","poster_card_width_dp",AJ6_INT,NULL,0},
 {AJ_RAIO_DP,"layout_settings","poster_card_corner_radius_dp",AJ6_INT,NULL,0},
 {AJ_CW_DIAS,"trakt_settings","continue_watching_days_cap",AJ6_INT,NULL,0},
 {AJ_RESOLVE_LIMIT,"debrid_settings","instant_playback_preparation_limit",AJ6_INT,NULL,0},
 {AJ_DESCOBRIR,"layout_settings","discover_location",AJ6_CHOICE,aj6Choice30,3},
 {AJ_NOTAS_HOME,"layout_settings","home_imdb_ratings_visibility",AJ6_CHOICE,aj6Choice31,2},
 {AJ_CW_ESTILO,"layout_settings","continue_watching_card_style",AJ6_CHOICE,aj6Choice32,3},
 {AJ_CW_ORDEM,"layout_settings","continue_watching_sort_mode",AJ6_CHOICE,aj6Choice33,3},
 {AJ_FONTE_ESCOPO,"player_settings","stream_auto_play_source",AJ6_CHOICE,aj6Choice34,3},
 {AJ_TEMA,"theme_settings","selected_theme",AJ6_CHOICE,aj6Choice35,12},
 {AJ_CW_FONTE,"trakt_settings","watch_progress_source",AJ6_CHOICE,aj6Choice36,4},
 {AJ_RESOLVE_PROVIDER,"debrid_settings","preferred_resolver_provider_id",AJ6_CHOICE,aj6Choice37,3},
 {AJ_LINK_PRAZO,"player_settings","stream_reuse_last_link_cache_hours",AJ6_HOURS,NULL,0},
 {AJ_LEG_MOSTRAR,"player_settings","subtitle_show_only_preferred_languages",AJ6_BOOL,NULL,0},
 {AJ_FOCO_TRAILER,"layout_settings","focused_poster_backdrop_trailer_enabled",AJ6_FOCUS,NULL,0},
};
