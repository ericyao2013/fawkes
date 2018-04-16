/****************************************************************************
 *  CLIPS Executive REST API -- Schema DomainPreconditionCompound
 *  (auto-generated, do not modify directly)
 *
 *  API Contact: Tim Niemueller <niemueller@kbsg.rwth-aachen.de>
 *  API Version: v1beta1
 *  API License: Apache 2.0
 ****************************************************************************/

import { DomainPrecondition } from './DomainPrecondition';
import { DomainPreconditionAtom } from './DomainPreconditionAtom';


/** DomainPreconditionCompound representation for JSON transfer. */
export interface DomainPreconditionCompound
{
	kind: string;
	apiVersion: string;
	name: string;
	type: DomainPreconditionCompound.TypeEnum;
	grounded: boolean;
	is_satisfied: boolean;
	elements: Array<DomainPrecondition | DomainPreconditionAtom | DomainPreconditionCompound>;
}

export namespace DomainPreconditionCompound
{
	export const API_VERSION: string = "v1beta1";

	export type TypeEnum = 'conjunction' | 'disjunction' | 'atom';
	export const TypeEnum = {
		conjunction: 'conjunction' as TypeEnum,
		disjunction: 'disjunction' as TypeEnum,
		atom: 'atom' as TypeEnum
	}
}