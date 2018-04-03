
// Copyright  2018  Tim Niemueller <niemueller@kbsg.rwth-aachen.de>
// License: Apache 2.0 (http://www.apache.org/licenses/LICENSE-2.0)

import { Component, OnInit, ViewChild } from '@angular/core';
import { Router, ActivatedRoute, ParamMap } from '@angular/router';
import { MatTableDataSource } from '@angular/material';

import { CardListFilterComponent } from '../../../components/filter/component';
import { ClipsApiService } from '../services/api.service';
import { Fact } from '../models/Fact';

import { Observable } from 'rxjs/Observable';
import 'rxjs/add/observable/interval';
import 'rxjs/add/operator/switchMap';
import { catchError, tap } from 'rxjs/operators';

@Component({
  selector: 'clips-env',
  templateUrl: './clips-env.component.html',
  styleUrls: ['./clips-env.component.scss']
})
export class ClipsEnvComponent implements OnInit {

  displayed_columns = ['index', 'formatted'];

  env: string = "";
  auto_refresh_subscription = null;
  loading = false;
  zero_message = "No facts received.";
  font_large = false;

  envs: string[] = [];

  data_source = new MatTableDataSource();

  @ViewChild(CardListFilterComponent) private readonly card_filter_: CardListFilterComponent;

  constructor(private readonly api_service: ClipsApiService,
              private route: ActivatedRoute,
              private router: Router)
  {
  }

  ngOnInit() {

    this.route.paramMap
      .subscribe((params: ParamMap) => {
        this.env = params.get('env');
        this.card_filter_.clearInput();
        this.refresh();
      });

    this.refresh_envs();

    this.card_filter_.filterEvent
      .subscribe((query: string) => {
        this.apply_filter(query)
      });
  }

  refresh()
  {
    this.loading = true;
    this.zero_message = "Retrieving facts";
    
    this.api_service.get_facts(this.env, false, true)
      .subscribe(
        (facts) => {
          this.data_source.data = facts;
          if (facts.length == 0) {
            this.zero_message = "No facts in fact base";
          }
          this.loading = false;
        },
        (err) => {
          this.data_source.data = [];
          if (err.status == 0) {
            this.zero_message="API server unavailable. Robot down?";
          } else {
            this.zero_message=`Failed to retrieve facts: ${err.error}`;
          }
          this.loading = false;
        }
      );
  }

  refresh_envs()
  {
    this.api_service.list_environments().subscribe(
      (envs) => {
        this.envs = envs.map(env => env.name);
      },
      (err) => {
        this.envs = [this.env]
      }
    );
  }

  private enable_autorefresh()
  {
    if (this.auto_refresh_subscription)  return;
    this.auto_refresh_subscription =
      Observable.interval(2000).subscribe((num) => {
        this.refresh();
      });
    this.refresh();
  }

  private disable_autorefresh()
  {
    if (this.auto_refresh_subscription) {
      this.auto_refresh_subscription.unsubscribe();
      this.auto_refresh_subscription = null;
    }
  }

  toggle_autorefresh()
  {
    if (this.auto_refresh_subscription) {
      this.disable_autorefresh();
    } else {
      this.enable_autorefresh();
    }
  }

  toggle_fontsize()
  {
    this.font_large = !this.font_large;
  }

  apply_filter(filter_value: string) {
    filter_value = filter_value.trim();
    // MatTableDataSource defaults to lowercase matches
    filter_value = filter_value.toLowerCase();
    this.data_source.filter = filter_value;
  }
}