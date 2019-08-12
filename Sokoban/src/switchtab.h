#ifndef SWITCHTAB_H
#define SWITCHTAB_H

#include "editortab.h"

class Switch;
class Switchable;
class Signaler;
class ThresholdSignaler;
class ParitySignaler;
class RoomMap;

enum class SignalerType {
	Threshold,
	Parity,
};

class SwitchTab: public EditorTab {
public:
    SwitchTab(EditorState*, GraphicsManager*);
    ~SwitchTab();

    void init();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);
    void handle_right_click(EditorRoom*, Point3);

    int get_signaler_labels(const char* labels[], std::vector<std::unique_ptr<Signaler>>& signalers);
	void threshold_signaler_options(ThresholdSignaler* t_sig, RoomMap* map);
	void parity_signaler_options(ParitySignaler* p_sig, RoomMap* map);

	void print_switch_list(std::vector<Switch*>* switch_group, const char* group_label);
	void print_switchable_list(std::vector<Switchable*>* switchable_group, const char* group_label);

private:
    std::vector<Switch*> model_switches_;
    std::vector<Switchable*> model_switchables_;
	std::vector<std::vector<Switchable*>> model_p_switchables_;

	std::vector<Switch*>* switches_ptr_;
	std::vector<Switchable*>* switchables_ptr_;
	std::vector<std::vector<Switchable*>>* p_switchables_ptr_;

	std::string model_label_;
    int model_threshold_;
	int model_parity_level_;
	
	int parity_index_;

	SignalerType sig_type_;
};

#endif // SWITCHTAB_H
